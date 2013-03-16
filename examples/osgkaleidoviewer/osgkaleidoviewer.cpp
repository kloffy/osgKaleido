#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osg/LightModel>
#include <osg/PolygonMode>
#include <osgGA/LambdaEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/Polyhedron>

#include <wild/conversion.hpp>

inline int mod(int x, int m)
{
	int r = x % m;
	return r<0 ? r+m : r;
}

void createPolyhedron(osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, int index)
{
	auto symbol = "#" + wild::conversion_cast<std::string>(mod(index, 80) + 1);

	polyhedron = new osgKaleido::Polyhedron(symbol);
}

void updatePolyhedron(osg::ref_ptr<osg::Geode>& geode, osg::ref_ptr<osg::Geometry>& geometry, osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, int faces)
{
	if (geometry) geode->removeDrawable(geometry);
	geometry = osgKaleido::createFaces(polyhedron.get(), static_cast<osgKaleido::Polyhedron::Faces>(faces));
	if (geometry) geode->addDrawable(geometry);
}

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
	osg::ref_ptr<osg::Geometry> geometry;
	osg::ref_ptr<osgKaleido::Polyhedron> polyhedron;

	osgViewer::Viewer viewer;
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	//viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	//viewer.setRunFrameScheme(osgViewer::Viewer::CONTINUOUS);
	//viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
	lightModel->setTwoSided(true);

	osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);

	auto stateSet = root->getOrCreateStateSet();
	stateSet->setAttributeAndModes(lightModel, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	osg::ref_ptr<osgGA::LambdaEventHandler> eventHandler = new osgGA::LambdaEventHandler;

	int faces = osgKaleido::Polyhedron::All;
	int index = 26;

	createPolyhedron(polyhedron, index);
	updatePolyhedron(geode, geometry, polyhedron, faces);

	eventHandler->onKeyDown([&](const osgGA::GUIEventAdapter& ea){
		auto key = ea.getKey();
		auto num = key - osgGA::GUIEventAdapter::KEY_0;
		if (0 <= num && num <= 9)
		{
			faces ^= (1 << mod(num-1, 10));
			updatePolyhedron(geode, geometry, polyhedron, faces);
			return true;
		}
		switch (key)
		{
		case osgGA::GUIEventAdapter::KEY_L:
		{
			lightModel->setTwoSided(!lightModel->getTwoSided());
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Right:
		{
			index++;
			faces = osgKaleido::Polyhedron::All;
			createPolyhedron(polyhedron, index);
			updatePolyhedron(geode, geometry, polyhedron, faces);
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Left:
		{
			index--;
			faces = osgKaleido::Polyhedron::All;
			createPolyhedron(polyhedron, index);
			updatePolyhedron(geode, geometry, polyhedron, faces);
			return true;
		}
		default:
			return false;
		}
	});

	root->addChild(geode);

	viewer.setSceneData(root.get());
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);
	viewer.addEventHandler(eventHandler);

	return viewer.run();
}