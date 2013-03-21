#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
//#include <osgGA/LambdaEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/PolyhedronGeode>

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
	viewer.realize();

	// Great ditrigonal icosidodecahedron
	osg::ref_ptr<osgKaleido::PolyhedronGeode> polyhedron = new osgKaleido::PolyhedronGeode("3/2|3 5");

	viewer.setSceneData(polyhedron.get());
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);

	return viewer.run();
}