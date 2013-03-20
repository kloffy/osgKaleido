#include <osgKaleido/PolyhedronGeode>

#pragma warning(push)
#pragma warning(disable: 4250)

#include <osgDB/Registry>
#include <osgUtil/Tessellator>

#pragma warning(pop)

namespace osgKaleido {

PolyhedronGeode::PolyhedronGeode(): _polyhedron(new Polyhedron), _faces(PolyhedronGeode::All), _dirty(true)
{
}

PolyhedronGeode::PolyhedronGeode(std::string const& symbol): _polyhedron(new Polyhedron(symbol)), _faces(PolyhedronGeode::All), _dirty(true)
{
}

PolyhedronGeode::PolyhedronGeode(PolyhedronGeode const& other, osg::CopyOp const& op): _polyhedron(other._polyhedron), _faces(other._faces), _dirty(true)
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

void PolyhedronGeode::traverse(osg::NodeVisitor& nv)
{
/*
	osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
	
	if (uv)
	{
		OSG_WARN << "Update!" << std::endl;
	}

	if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
	{
		OSG_WARN << "Update!" << std::endl;
	}
*/
	if (_dirty)
	{
		OSG_WARN << "Update!" << std::endl;

		generate();
		_dirty = false;
	}

	Geode::traverse(nv);
}

void PolyhedronGeode::generate()
{
	if (_geometry) removeDrawable(_geometry);

	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);

	_geometry = createGeometry(*_polyhedron, static_cast<osgKaleido::Polyhedron::Faces>(_faces));

	//_geometry->dirtyBound();
	//_geometry->dirtyDisplayList();
	_geometry->setUseVertexBufferObjects(true);

	tessellator.retessellatePolygons(*_geometry);

	if (_geometry) addDrawable(_geometry);
}

}

REGISTER_OBJECT_WRAPPER(PolyhedronGeode, new osgKaleido::PolyhedronGeode, osgKaleido::PolyhedronGeode, "osg::Geode osgKaleido::PolyhedronGeode")
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