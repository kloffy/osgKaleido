#include <osgKaleido/PolyhedronGeometry>

#include <osgKaleido/Conversion>
#include <osgKaleido/Util>

#pragma warning(push)
#pragma warning(disable: 4250)

#include <osgDB/Registry>
#include <osgUtil/Tessellator>

#pragma warning(pop)

#include <cassert>

#include <wild/bits.hpp>

namespace osgKaleido {
namespace detail {

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

} // detail

/**
 * faces = pow(2, sides-3)
 */
PolyhedronGeometry::FaceMask PolyhedronGeometry::FaceMaskFromSides(int sides)
{
	assert(sides >= 3);
	return static_cast<PolyhedronGeometry::FaceMask>(1 << (sides - 3));
}

/**
 * sides = log(2, faces)+3
 */
int PolyhedronGeometry::SidesFromFaceMask(PolyhedronGeometry::FaceMask faces)
{
	assert(faces >= 1);
	return wild::fls(faces) + 3;
}

PolyhedronGeometry::PolyhedronGeometry():
	DynamicGeometry(),
	_symbol("#1"),
	_faceMask(FaceMask::All),
	_polyhedron(nullptr)
{
}

PolyhedronGeometry::PolyhedronGeometry(std::string const& symbol, FaceMask faceMask):
	DynamicGeometry(),
	_symbol(symbol),
	_faceMask(faceMask),
	_polyhedron(nullptr)
{
}

PolyhedronGeometry::PolyhedronGeometry(PolyhedronGeometry const& other, osg::CopyOp const& op):
	DynamicGeometry(other, op),
	_symbol(other._symbol),
	_faceMask(other._faceMask),
	_polyhedron(nullptr)
{
}

PolyhedronGeometry::~PolyhedronGeometry()
{
}

std::string const& PolyhedronGeometry::getSymbol() const
{
	return _symbol;
}

void PolyhedronGeometry::setSymbol(std::string const& symbol)
{
	_symbol = symbol;

	_polyhedron = nullptr;

	dirty();
}

PolyhedronGeometry::FaceMask PolyhedronGeometry::getFaceMask() const
{
	return _faceMask;
}

void PolyhedronGeometry::setFaceMask(PolyhedronGeometry::FaceMask faceMask)
{
	_faceMask = faceMask;

	dirty();
}

Polyhedron const* PolyhedronGeometry::getOrCreatePolyhedron()
{
	if (_polyhedron == nullptr)
	{
		_polyhedron = tryCreateOrReturnNull<Polyhedron>(_symbol);
	}

	return _polyhedron.get();
}

osg::Vec3Array* PolyhedronGeometry::getOrCreateVertexArray()
{
	auto result = dynamic_cast<osg::Vec3Array*>(getVertexArray());

	if (result == nullptr)
	{
		result = new osg::Vec3Array;
		setVertexArray(result);
	}

	return result;
}

osg::Vec3Array* PolyhedronGeometry::getOrCreateNormalArray()
{
	auto result = dynamic_cast<osg::Vec3Array*>(getNormalArray());

	if (result == nullptr)
	{
		result = new osg::Vec3Array;
		setNormalArray(result);
		setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	return result;
}

osg::Vec4Array* PolyhedronGeometry::getOrCreateColorArray()
{
	auto result = dynamic_cast<osg::Vec4Array*>(getColorArray());

	if (result == nullptr)
	{
		result = new osg::Vec4Array;
		setColorArray(result);
		setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	return result;
}

void PolyhedronGeometry::updateImplementation(osg::NodeVisitor* nv)
{
	OSG_WARN << "Update!" << std::endl;

	removePrimitiveSet(0, getNumPrimitiveSets());

	auto _vertices = getOrCreateVertexArray();
	auto _normals = getOrCreateNormalArray();
	auto _colors = getOrCreateColorArray();

	_vertices->clear();
	_normals->clear();
	_colors->clear();

	auto polyhedron = getOrCreatePolyhedron();

	if (polyhedron == nullptr) return;

	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);

	osg::ref_ptr<osg::Vec3Array> vertices = createVertexArray(*polyhedron);
	VertexIndexArrays polygons = createVertexIndexArrays(*polyhedron);

	for (auto const& polygon: polygons)
	{
		assert(polygon && polygon->size() >= 3);

		if (!(_faceMask & PolyhedronGeometry::FaceMaskFromSides(polygon->size()))) continue;

		auto first = _vertices->size();
		auto count = polygon->size();

		auto normal = detail::calculateNormal(vertices, polygon);
		auto color = detail::calculateColor(vertices, polygon);

		for (auto i = 0u; i < polygon->size(); ++i)
		{
			auto vertex = vertices->at(polygon->at(i));
			_vertices->push_back(vertex);
			_normals->push_back(normal);
			_colors->push_back(color);
		}

		addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, first, count));
	}

	_vertices->dirty();
	_normals->dirty();
	_colors->dirty();

	tessellator.retessellatePolygons(*this);
}

} // osgKaleido

REGISTER_OBJECT_WRAPPER(PolyhedronGeometry, new osgKaleido::PolyhedronGeometry, osgKaleido::PolyhedronGeometry, "osg::Geometry osgKaleido::PolyhedronGeometry")
{
	ADD_STRING_SERIALIZER(Symbol, "#1");

	BEGIN_ENUM_SERIALIZER2(FaceMask, osgKaleido::PolyhedronGeometry::FaceMask, All);
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
}