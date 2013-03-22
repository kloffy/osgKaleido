#include <osgKaleido/DynamicGeometry>

namespace osgKaleido {

void DynamicGeometry::Callback::update(osg::NodeVisitor* nv, osg::Drawable* drawable)
{
	static_cast<DynamicGeometry*>(drawable)->update(nv);
}

DynamicGeometry::DynamicGeometry():
	osg::Geometry(),
	_callback(new Callback),
	_dirty(true)
{
	setDataVariance(osg::Object::DYNAMIC);
	setUpdateCallback(_callback);
}

DynamicGeometry::DynamicGeometry(DynamicGeometry const& other, osg::CopyOp const& op):
	osg::Geometry(other, op),
	_callback(new Callback),
	_dirty(true)
{
	setDataVariance(osg::Object::DYNAMIC);
	setUpdateCallback(_callback);
}

DynamicGeometry::~DynamicGeometry()
{
}

void DynamicGeometry::update(osg::NodeVisitor* nv)
{
	if (isDirty())
	{
		clean();
		updateImplementation(nv);
	}
}

} // osgKaleido