#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osg/LightModel>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/PolyhedronGeometry>

int main(int argc, char** argv)
{
	//osg::setNotifyLevel(osg::INFO);

	osg::ArgumentParser arguments(&argc, argv);

	unsigned int windowWidth = 800, windowHeight = 600;

	while (arguments.read("--width", windowWidth));
	while (arguments.read("--height", windowHeight));

	osg::observer_ptr<osg::GraphicsContext::WindowingSystemInterface> wsi = osg::GraphicsContext::getWindowingSystemInterface(); 
	if (!wsi)
	{
		OSG_WARN << "No WindowingSystemInterface available." <<std::endl; 
		return 1; 
	}
	osg::GraphicsContext::ScreenSettings screenSettings;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(0), screenSettings);

	osgViewer::Viewer viewer(arguments);
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	osg::LightModel* lightModel = new osg::LightModel;  
	lightModel->setTwoSided(true);

	osg::ref_ptr<osg::Geode> root = new osg::Geode;

	auto stateSet = root->getOrCreateStateSet();
	stateSet->setAttributeAndModes(lightModel, osg::StateAttribute::ON);

	osg::ref_ptr<osgKaleido::PolyhedronGeometry> geometry = new osgKaleido::PolyhedronGeometry("3/2|3 5");
	//osg::ref_ptr<osgKaleido::PolyhedronGeometry> geometry = new osgKaleido::PolyhedronGeometry("#80");

	auto polyhedron = geometry->getOrCreatePolyhedron();
	OSG_WARN << polyhedron->getName() << std::endl;
	OSG_WARN << polyhedron->getWythoffSymbol() << std::endl;
	OSG_WARN << polyhedron->getVertexConfiguration() << std::endl;

	root->addDrawable(geometry);

	viewer.setSceneData(root.get());

	return viewer.run();
}