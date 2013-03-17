#include <osgKaleido/Polyhedron>

#include <osgKaleido/Conversion>
#include <osgKaleido/Util>

#include <osgUtil/Tessellator>

#include <ckaleido>

#include <wild/math.hpp>
#include <wild/bits.hpp>

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

::Vector polyhedron_face_color(::Polyhedron const* P, int ftype)
{
	::Vector result;

	auto sides = polyhedron_ftype_sides(P, ftype);
	auto x = sides - 2;

	result.x = std::max(0.25f, static_cast<float>((x >> 0) & 1));
	result.y = std::max(0.25f, static_cast<float>((x >> 1) & 1));
	result.z = std::max(0.25f, static_cast<float>((x >> 2) & 1));

	return result;
}

int polyhedron_count_faces(::Polyhedron const* P, osgKaleido::Polyhedron::Faces faces)
{
	int result = 0;
	for(int n = 0; n < P->N; ++n)
	{
		auto sides = polyhedron_ftype_sides(P, n);
		if(faces & Polyhedron::sidesToFace(sides))
		{
			result += P->m[n];
		}
	}
	return result;
}

}

/**
 * faces = pow(2, sides-3)
 */
Polyhedron::Faces Polyhedron::sidesToFace(int sides)
{
	assert(sides > 2);
	return static_cast<Polyhedron::Faces>(1 << (sides - 3));
}

/**
 * sides = log(2, faces)+3
 */
int Polyhedron::faceToSides(Polyhedron::Faces faces)
{
	assert(faces > 0);
	return wild::fls(faces) + 3;
}

Polyhedron::Polyhedron(std::string const& symbol): _symbol(symbol), _data(nullptr)
{
	OSG_DEBUG << "Create Polyhedron" << std::endl;

	int need_coordinates = 1, need_edgelist = 1, need_approx = 0, just_list = 0;

	_data = kaleido(const_cast<char*>(_symbol.c_str()), need_coordinates, need_edgelist, need_approx, just_list);

	checkStatusAndThrowException(*this);
}

Polyhedron::~Polyhedron()
{
	OSG_DEBUG << "Delete Polyhedron" << std::endl;

	polyfree(_data);
}

std::string const& Polyhedron::getSymbol() const
{
	return _symbol;
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

std::size_t Polyhedron::getFaceCount(Faces faces) const
{
	return detail::polyhedron_count_faces(_data, faces);
}

bool Polyhedron::isOneSided() const
{
	return _data->onesided != 0;
}

bool Polyhedron::isHemi() const
{
	return _data->hemi != 0;
}

osg::Vec3Array* createVertices(Polyhedron const* polyhedron)
{
	auto P = polyhedron->getData();

	osg::ref_ptr<osg::Vec3Array> result = new osg::Vec3Array;
	for (int v = 0; v < P->V; ++v)
	{
		result->push_back(wild::conversion_cast<osg::Vec3d>(P->v[v]));
	}
	return result.release();
}

osg::Geometry* createFaces(Polyhedron const* polyhedron, Polyhedron::Faces faces)
{
	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);

	osg::ref_ptr<osg::Geometry> result = new osg::Geometry;

	osg::Vec3Array* vertices = new osg::Vec3Array;
	osg::Vec3Array* normals = new osg::Vec3Array;
	osg::Vec4Array* colors = new osg::Vec4Array;

	result->setVertexArray(vertices);

	result->setNormalArray(normals);
	result->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	result->setColorArray(colors);
	result->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	auto P = polyhedron->getData();

	for (int f = 0; f < P->F; f++)
	{
		auto ftype = P->ftype[f];
		auto sides = detail::polyhedron_ftype_sides(P, ftype);
		auto face = Polyhedron::sidesToFace(sides);

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

		auto a = vertices->at(vertices->size()-1);
		auto b = vertices->at(vertices->size()-2);
		auto c = vertices->at(vertices->size()-3);
		auto normal = (a-b) ^ (c-b);
		normal.normalize();
		auto color = wild::conversion_cast<osg::Vec4d>(detail::polyhedron_face_color(P, ftype));
		for (auto i=0u; i < count; ++i)
		{
			normals->push_back(normal);
			colors->push_back(color);
		}
	}

	tessellator.retessellatePolygons(*result);

	return result.release();
}

} // osgKaleido