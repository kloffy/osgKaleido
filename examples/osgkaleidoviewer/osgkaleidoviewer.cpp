#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/Polyhedron>

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc,argv);

	osg::observer_ptr<osg::GraphicsContext::WindowingSystemInterface> wsi = osg::GraphicsContext::getWindowingSystemInterface(); 
	if (!wsi)
	{
		OSG_NOTICE << "Error: No WindowingSystemInterface available." <<std::endl; 
		return 1; 
	}

	unsigned int windowWidth = 800, windowHeight = 600;

	while (arguments.read("--width", windowWidth));
	while (arguments.read("--height", windowHeight));

	osg::GraphicsContext::ScreenSettings screenSettings;
	wsi->getScreenSettings(osg::GraphicsContext::ScreenIdentifier(0), screenSettings);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	osgViewer::Viewer viewer;
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	//viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	//viewer.setRunFrameScheme(osgViewer::Viewer::CONTINUOUS);
	//viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	osg::ref_ptr<osgKaleido::Polyhedron> p = new osgKaleido::Polyhedron("#10");

	OSG_WARN << osgKaleido::Polyhedron::Faces::All << std::endl;

	geode->addDrawable(p.get());

	root->addChild(geode);

	viewer.setSceneData(root.get());
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);

	return viewer.run();
}