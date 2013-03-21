#include <osgKaleido/DynamicGeode>

namespace osgKaleido {
	
void DynamicGeode::Callback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	DynamicGeode* dynamicGeode = static_cast<DynamicGeode*>(node);

	if (dynamicGeode->isDirty())
	{
		dynamicGeode->clean();
		dynamicGeode->update(nv);
	}
}

DynamicGeode::DynamicGeode():
	osg::Geode(),
	_callback(new Callback),
	_dirty(true)
{
	addUpdateCallback(_callback);
}

DynamicGeode::DynamicGeode(DynamicGeode const& other, osg::CopyOp const& op):
	osg::Geode(other, op),
	_callback(new Callback),
	_dirty(true)
{
	addUpdateCallback(_callback);
}

DynamicGeode::~DynamicGeode()
{
}

} // osgKaleido