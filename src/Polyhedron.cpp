#include <osgKaleido/Polyhedron>

#include <osgKaleido/Conversion>
#include <osgKaleido/Util>

#pragma warning(push)
#pragma warning(disable: 4250)

#include <osgDB/Registry>
#include <osgUtil/Tessellator>

#pragma warning(pop)

#include <ckaleido>

#include <wild/math.hpp>

namespace osgKaleido {
namespace detail {

/**
 * Check if the vertex belongs to a given face.
 */
int polyhedron_face_contains_vertex(::Polyhedron const* P, int f, int v)
{
	for (int m = 0; m < P->M; ++m) 
	{
		if (P->incid[m][v] == f) return m;
	}
	return -1;
}

/**
 * Search all vertices of for a vertex belonging to a given face.
 */
int polyhedron_face_first_vertex(::Polyhedron const* P, int f, int& k)
{
	for (int v = 0; v < P->V; ++v)
	{
		if ((k = polyhedron_face_contains_vertex(P, f, v)) != -1) return v;
	}
	return -1;
}

/**
 * Search vertices adjacent to the current vertex for another vertex belonging to a given face.
 */
int polyhedron_face_next_vertex(::Polyhedron const* P, int f, int& k, int cv, int pv)
{
	for (int m = 0; m < P->M; ++m)
	{
		int v = P->adj[wild::mod(k+m, P->M)][cv];
		if (v == pv) continue;
		if ((k = polyhedron_face_contains_vertex(P, f, v)) != -1) return v;
	}
	return -1;
}

int polyhedron_ftype_sides(::Polyhedron const* P, int ftype)
{
	return numerator(P->n[ftype]);
}
/*
osg::Vec4 face_color(osgKaleido::Polyhedron::Faces faces)
{
	osg::Vec4 result;

	auto x = Polyhedron::faceToSides(faces) - 2;

	result.r() = std::max(0.25f, static_cast<float>((x >> 0) & 1));
	result.g() = std::max(0.25f, static_cast<float>((x >> 1) & 1));
	result.b() = std::max(0.25f, static_cast<float>((x >> 2) & 1));
	result.a() = 1.0f;

	return result;
}

std::size_t face_count(osgKaleido::Polyhedron::Faces faces, ::Polyhedron const* P)
{
	std::size_t result = 0;
	for(int n = 0; n < P->N; ++n)
	{
		auto face = Polyhedron::sidesToFace(detail::polyhedron_ftype_sides(P, n));

		if (!(faces & face)) continue;

		result += P->m[n];
	}
	return result;
}
*/
}

Polyhedron::Polyhedron(): _symbol("#1"), _data(nullptr)
{
	create();
}

Polyhedron::Polyhedron(std::string const& symbol): _symbol(symbol), _data(nullptr)
{
	create();
}

Polyhedron::Polyhedron(Polyhedron const& other, osg::CopyOp const& op): _symbol(other._symbol), _data(nullptr)
{
	create();
}

Polyhedron::~Polyhedron()
{
	destroy();
}

void Polyhedron::create()
{
	OSG_DEBUG << "Create Polyhedron" << std::endl;

	int need_coordinates = 1, need_edgelist = 1, need_approx = 0, just_list = 0;

	_data = kaleido(const_cast<char*>(_symbol.c_str()), need_coordinates, need_edgelist, need_approx, just_list);

	checkStatusAndThrowException(*this);
}

void Polyhedron::destroy()
{
	OSG_DEBUG << "Delete Polyhedron" << std::endl;

	if (_data != nullptr)
	{
		polyfree(_data);
		_data = nullptr;
	}
}

std::string const& Polyhedron::getSymbol() const
{
	return _symbol;
}

void Polyhedron::setSymbol(std::string const& symbol)
{
	destroy();

	_symbol = symbol;

	create();
} 

std::string Polyhedron::getName() const
{
	return wild::conversion_cast<std::string>(_data->name);
}

std::string Polyhedron::getDualName() const
{
	return wild::conversion_cast<std::string>(_data->dual_name);
}

std::string Polyhedron::getWythoffSymbol() const
{
	return wild::conversion_cast<std::string>(_data->polyform);
}

std::string Polyhedron::getVertexConfiguration() const
{
	return wild::conversion_cast<std::string>(_data->config);
}

std::size_t Polyhedron::getVertexCount() const
{
	return _data->V;
}

std::size_t Polyhedron::getFaceCount() const
{
	return _data->F;
}

bool Polyhedron::isOneSided() const
{
	return _data->onesided != 0;
}

bool Polyhedron::isHemi() const
{
	return _data->hemi != 0;
}

osg::Vec3Array* createVertexArray(Polyhedron const& polyhedron)
{
	osg::ref_ptr<osg::Vec3Array> result = new osg::Vec3Array;

	auto P = polyhedron.getData();
	for (int v = 0; v < P->V; ++v)
	{
		result->push_back(wild::conversion_cast<osg::Vec3d>(P->v[v]));
	}

	return result.release();
}

osg::UShortArray* createVertexIndexArray(Polyhedron const& polyhedron, int f)
{
	osg::ref_ptr<osg::UShortArray> result = new osg::UShortArray;

	auto P = polyhedron.getData();

	// Using this as an offset into the adjacency matrix (mostly) ensures correct winding.
	int magic = 0;
	// fv, pv, cv, nv
	int fv = detail::polyhedron_face_first_vertex(P, f, magic);
	int nv = fv, pv = fv;
	do
	{
		int cv = nv;
		result->push_back(cv);
		nv = detail::polyhedron_face_next_vertex(P, f, magic, cv, pv);
		pv = cv;
		if (fv == nv) break;
	}
	while (true);

	return result.release();
}

VertexIndexArrays createVertexIndexArrays(Polyhedron const& polyhedron)
{
	VertexIndexArrays result;

	auto P = polyhedron.getData();
	for (int f = 0; f < P->F; f++)
	{
		result.push_back(createVertexIndexArray(polyhedron, f));
	}

	return result;
}

/*
void createFaces(osg::Geometry* result, Polyhedron const* polyhedron, Polyhedron::Faces faces)
{
	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);

	osg::Vec3Array* vertices = new osg::Vec3Array;
	osg::Vec3Array* normals = new osg::Vec3Array;
	osg::Vec4Array* colors = new osg::Vec4Array;

	result->removePrimitiveSet(0, result->getNumPrimitiveSets());

	auto P = polyhedron->getData();

	for (int f = 0; f < P->F; f++)
	{
		auto ftype = P->ftype[f];
		auto face = Polyhedron::sidesToFace(detail::polyhedron_ftype_sides(P, ftype));
		
		if (!(faces & face)) continue;

		auto size = vertices->size();

		// Using this as an offset into the adjacency matrix (mostly) ensures correct winding.
		int magic = 0;
		// fv, pv, cv, nv
		int fv = detail::polyhedron_face_first_vertex(P, f, magic);
		int nv = fv, pv = fv;
		do
		{
			int cv = nv;
			vertices->push_back(wild::conversion_cast<osg::Vec3d>(P->v[cv]));
			nv = detail::polyhedron_face_next_vertex(P, f, magic, cv, pv);
			pv = cv;
			if (fv == nv) break;
		}
		while (true);

		auto count = vertices->size() - size;
		result->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, size, count));

		auto const& a = vertices->at(vertices->size()-1);
		auto const& b = vertices->at(vertices->size()-2);
		auto const& c = vertices->at(vertices->size()-3);
		auto normal = (a-b) ^ (c-b);
		normal.normalize();

		auto color = detail::face_color(face);
		
		for (auto i=0u; i < count; ++i)
		{
			normals->push_back(normal);
			colors->push_back(color);
		}
	}

	result->setVertexArray(vertices);

	result->setNormalArray(normals);
	result->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	result->setColorArray(colors);
	result->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	tessellator.retessellatePolygons(*result);
}
*/

} // osgKaleido

REGISTER_OBJECT_WRAPPER(Polyhedron, new osgKaleido::Polyhedron, osgKaleido::Polyhedron, "osg::Object osgKaleido::Polyhedron")
{
	ADD_STRING_SERIALIZER(Symbol, "#1");
}