#include <osgKaleido/Polyhedron>

#include <osgKaleido/Conversion>
#include <osgKaleido/Util>

#include <osgUtil/Tessellator>

namespace osgKaleido {
namespace detail {

/*
 * Check if the vertex belongs to a given face.
 */
int polyhedron_face_contains_vertex(::Polyhedron *P, int f, int v)
{
	for (int m = 0; m < P->M; ++m) 
	{
		if (P->incid[m][v] == f) return m;
	}
	return -1;
}

/*
 * Search all vertices of for a vertex belonging to a given face.
 */
int polyhedron_face_first_vertex(::Polyhedron *P, int f, int& k)
{
	for (int v = 0; v < P->V; ++v)
	{
		if ((k = polyhedron_face_contains_vertex(P, f, v)) != -1) return v;
	}
	return -1;
}

/*
 * Search vertices adjacent to the current vertex for another vertex belonging to a given face.
 */
int polyhedron_face_next_vertex(::Polyhedron *P, int f, int& k, int cv, int pv)
{
	for (int m = 0; m < P->M; ++m)
	{
		int v = P->adj[mod(k+m, P->M)][cv];
		if (v == pv) continue;
		if ((k = polyhedron_face_contains_vertex(P, f, v)) != -1) return v;
	}
	return -1;
}

int polyhedron_ftype_sides(::Polyhedron* P, int ftype)
{
	return numerator(P->n[ftype]);
}

Polyhedron::Faces polyhedron_ftype_face(::Polyhedron* P, int ftype)
{
	int sides = polyhedron_ftype_sides(P, ftype);
	return static_cast<Polyhedron::Faces>(1 << (sides - 3));
}

::Vector polyhedron_face_color(Polyhedron::Faces face)
{
	::Vector result;

	int x = static_cast<int>(face);

	result.x = std::max(0.25f, static_cast<float>((x >> 0) & 1));
	result.y = std::max(0.25f, static_cast<float>((x >> 1) & 1));
	result.z = std::max(0.25f, static_cast<float>((x >> 2) & 1));

	return result;
}

int polyhedron_count_faces(::Polyhedron* P, Polyhedron::Faces faces)
{
	int result = 0;
	for(int n = 0; n < P->N; ++n)
	{
		if(faces & polyhedron_ftype_face(P, n))
		{
			result += P->m[n];
		}
	}
	return result;
}

}

Polyhedron::Polyhedron(std::string const& symbol): _symbol(symbol), _data(nullptr)
{
	int need_coordinates = 1, need_edgelist = 1, need_approx = 0, just_list = 0;

	_data = kaleido(const_cast<char*>(_symbol.c_str()), need_coordinates, need_edgelist, need_approx, just_list);

	if (validate_or_throw(*this))
	{
		update();
	}
}

Polyhedron::~Polyhedron()
{
	polyfree(_data);
}

void Polyhedron::update()
{
	_name = _data->name;
	_configuration = _data->config;
}

std::string const& Polyhedron::getName() const
{
	return _name;
}

std::string const& Polyhedron::getConfiguration() const
{
	return _configuration;
}

int Polyhedron::getFaceCount(Faces faces)
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
		auto face = detail::polyhedron_ftype_face(P, P->ftype[f]);

		if (!(faces & face)) continue;

		auto size = vertices->size();

		int magic = 0;
		int fv = detail::polyhedron_face_first_vertex(P, f, magic);
		int nv = fv, pv = fv;
		do
		{
			int v = nv;
			vertices->push_back(wild::conversion_cast<osg::Vec3d>(P->v[v]));
			nv = detail::polyhedron_face_next_vertex(P, f, magic, v, pv);
			if (fv == nv) break;
			pv = v;
		}
		while (true);

		auto count = vertices->size() - size;
		result->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, size, count));

		auto a = vertices->at(vertices->size()-1);
		auto b = vertices->at(vertices->size()-2);
		auto c = vertices->at(vertices->size()-3);
		auto normal = (a-b) ^ (c-b);
		normal.normalize();
		auto color = wild::conversion_cast<osg::Vec4d>(detail::polyhedron_face_color(face));
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