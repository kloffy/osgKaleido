#include <osgKaleido/PolyhedronGeode>

#pragma warning(push)
#pragma warning(disable: 4250)

#include <osgDB/Registry>
#include <osgUtil/Tessellator>

#pragma warning(pop)

#include <cassert>

#include <wild/bits.hpp>

namespace osgKaleido {

/**
 * faces = pow(2, sides-3)
 */
PolyhedronGeode::FaceMask PolyhedronGeode::FaceMaskFromSides(int sides)
{
	assert(sides > 2);
	return static_cast<PolyhedronGeode::FaceMask>(1 << (sides - 3));
}

/**
 * sides = log(2, faces)+3
 */
int PolyhedronGeode::SidesFromFaceMask(PolyhedronGeode::FaceMask faces)
{
	assert(faces > 0);
	return wild::fls(faces) + 3;
}

PolyhedronGeode::PolyhedronGeode():
	DynamicGeode(),
	_polyhedron(new Polyhedron()),
	_faces(PolyhedronGeode::All)
{
}

PolyhedronGeode::PolyhedronGeode(std::string const& symbol):
	DynamicGeode(),
	_polyhedron(new Polyhedron(symbol)),
	_faces(PolyhedronGeode::All)
{
}

PolyhedronGeode::PolyhedronGeode(PolyhedronGeode const& other, osg::CopyOp const& op):
	DynamicGeode(other, op),
	_polyhedron(other._polyhedron), 
	_faces(other._faces)
{
}

PolyhedronGeode::~PolyhedronGeode()
{
}

std::string const& PolyhedronGeode::getSymbol() const
{
	return _polyhedron->getSymbol();
}

void PolyhedronGeode::setSymbol(std::string const& symbol)
{
	_polyhedron->setSymbol(symbol);

	dirty();
}

PolyhedronGeode::FaceMask PolyhedronGeode::getFaceMask() const
{
	return _faces;
}

void PolyhedronGeode::setFaceMask(PolyhedronGeode::FaceMask faces)
{
	_faces = faces;

	dirty();
}

void PolyhedronGeode::update(osg::NodeVisitor* nv)
{
	if (_geometry) removeDrawable(_geometry);

	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);

	_geometry = createGeometry(*_polyhedron, _faces);

	//_geometry->dirtyBound();
	//_geometry->dirtyDisplayList();

	_geometry->setUseVertexBufferObjects(true);

	tessellator.retessellatePolygons(*_geometry);

	if (_geometry) addDrawable(_geometry);
}


osg::Vec3 calculateNormal(osg::Vec3Array* vertices, osg::UShortArray* polygon)
{
	assert(polygon && polygon->size() >= 3);

	auto const& a = vertices->at(polygon->at(2));
	auto const& b = vertices->at(polygon->at(1));
	auto const& c = vertices->at(polygon->at(0));

	auto normal = (a-b) ^ (c-b);
	normal.normalize();

	return normal;
}

osg::Vec4 calculateColor(osg::Vec3Array* vertices, osg::UShortArray* polygon)
{
	assert(polygon && polygon->size() >= 3);

	osg::Vec4 result;

	auto x = polygon->size() - 2;

	result.r() = std::max(0.25f, static_cast<float>((x >> 0) & 1));
	result.g() = std::max(0.25f, static_cast<float>((x >> 1) & 1));
	result.b() = std::max(0.25f, static_cast<float>((x >> 2) & 1));
	result.a() = 1.0f;

	return result;
}

osg::Geometry* createBasicGeometry(Polyhedron const& polyhedron, PolyhedronGeode::FaceMask faces)
{
	osg::ref_ptr<osg::Geometry> result = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertices = createVertexArray(polyhedron);
	VertexIndexArrays polygons = createVertexIndexArrays(polyhedron);

	result->setVertexArray(vertices);

	for (auto const& polygon: polygons)
	{
		assert(polygon && polygon->size() >= 3);

		if (!(faces & PolyhedronGeode::FaceMaskFromSides(polygon->size()))) continue;

		result->addPrimitiveSet(new osg::DrawElementsUShort(osg::PrimitiveSet::POLYGON, polygon->size(), &polygon->front()));
	}

	return result.release();
}

osg::Geometry* createGeometry(Polyhedron const& polyhedron, PolyhedronGeode::FaceMask faces)
{
	osg::ref_ptr<osg::Geometry> result = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertices = createVertexArray(polyhedron);
	VertexIndexArrays polygons = createVertexIndexArrays(polyhedron);
	
	osg::ref_ptr<osg::Vec3Array> _vertices = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> _normals = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> _colors = new osg::Vec4Array;

	for (auto const& polygon: polygons)
	{
		assert(polygon && polygon->size() >= 3);

		if (!(faces & PolyhedronGeode::FaceMaskFromSides(polygon->size()))) continue;

		auto first = _vertices->size();
		auto count = polygon->size();

		auto normal = calculateNormal(vertices, polygon);
		auto color = calculateColor(vertices, polygon);

		for (auto i = 0u; i < polygon->size(); ++i)
		{
			auto vertex = vertices->at(polygon->at(i));
			_vertices->push_back(vertex);
			_normals->push_back(normal);
			_colors->push_back(color);
		}

		result->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, first, count));
	}

	result->setVertexArray(_vertices);

	result->setNormalArray(_normals);
	result->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

	result->setColorArray(_colors);
	result->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	return result.release();
}

} // osgKaleido

REGISTER_OBJECT_WRAPPER(PolyhedronGeode, new osgKaleido::PolyhedronGeode, osgKaleido::PolyhedronGeode, "osg::Geode osgKaleido::DynamicGeode osgKaleido::PolyhedronGeode")
{
	BEGIN_ENUM_SERIALIZER2(FaceMask, osgKaleido::PolyhedronGeode::FaceMask, All);
		ADD_ENUM_VALUE(None);
		ADD_ENUM_VALUE(Triangular);
		ADD_ENUM_VALUE(Quadrilateral);
		ADD_ENUM_VALUE(Pentagonal);
		ADD_ENUM_VALUE(Hexagonal);
		ADD_ENUM_VALUE(Heptagonal);
		ADD_ENUM_VALUE(Octagonal);
		ADD_ENUM_VALUE(Nonagonal);
		ADD_ENUM_VALUE(Decagonal);
		ADD_ENUM_VALUE(Hendecagonal);
		ADD_ENUM_VALUE(Dodecagonal);
		ADD_ENUM_VALUE(All);
	END_ENUM_SERIALIZER();

	ADD_OBJECT_SERIALIZER(Polyhedron, osgKaleido::Polyhedron, nullptr);
}