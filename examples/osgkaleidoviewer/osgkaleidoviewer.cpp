#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osg/LightModel>
#include <osg/PolygonMode>
#include <osgDB/ReadFile>
#include <osgGA/LambdaEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/Polyhedron>

#include <wild/conversion.hpp>
#include <wild/math.hpp>
#include <wild/bits.hpp>

void createPolyhedron(osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, int index)
{
	auto symbol = "#" + wild::conversion_cast<std::string>(wild::mod(index, 80) + 1);

	polyhedron = new osgKaleido::Polyhedron(symbol);

	OSG_INFO << polyhedron->getName() << " (" << polyhedron->getDualName() << "*)" << std::endl;
	OSG_INFO << polyhedron->getWythoffSymbol() << std::endl;
	OSG_INFO << polyhedron->getVertexConfiguration() << std::endl;
}

void updatePolyhedron(osg::ref_ptr<osg::Geode>& geode, osg::ref_ptr<osg::Geometry>& geometry, osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, int faces)
{
	if (geometry) geode->removeDrawable(geometry);
	geometry = osgKaleido::createFaces(polyhedron.get(), static_cast<osgKaleido::Polyhedron::Faces>(faces));
	geometry->setUseVertexBufferObjects(true);
	if (geometry) geode->addDrawable(geometry);
}

osg::Uniform* createUniformArray(std::string const& name, osg::Vec3Array* data)
{
	osg::Uniform* result = new osg::Uniform(osg::Uniform::FLOAT_VEC3, name, data->size());
	for (auto i = 0u; i < data->size(); ++i)
	{
		result->setElement(i, data->at(i));
	}
	return result;
}

int main(int argc, char** argv)
{
/*
	osg::ref_ptr<osg::UIntArray> array = new osg::UIntArray(); 
	array->push_back( 0 ); 
	array->push_back( 1 ); 
	array->push_back( 2 ); 
	array->push_back( 3 ); 
	p_geometry->addPrimitiveSet( new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, array->size(), & array->front() ) ); 
*/
	osg::setNotifyLevel(osg::INFO);

	//OSG_WARN << osgDB::getCurrentWorkingDirectory() << std::endl;

	osg::ArgumentParser arguments(&argc, argv);

	osg::observer_ptr<osg::GraphicsContext::WindowingSystemInterface> wsi = osg::GraphicsContext::getWindowingSystemInterface(); 
	if (!wsi)
	{
		OSG_WARN << "No WindowingSystemInterface available." <<std::endl; 
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

	osgViewer::Viewer viewer(arguments);
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	//viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	//viewer.setRunFrameScheme(osgViewer::Viewer::CONTINUOUS);
	//viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
	lightModel->setTwoSided(true);

	osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
	polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);

	auto state = viewer.getCamera()->getGraphicsContext()->getState();
	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);

	osg::ref_ptr<osg::Shader> vs = osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_directional2_vs.glsl"); 
	osg::ref_ptr<osg::Shader> fs = osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_directional2_fs.glsl"); 

	osg::Vec3f lightDir(0.0f, 0.0f, 1.0f);
    lightDir.normalize();

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(vs);
	program->addShader(fs);

	auto stateSet = root->getOrCreateStateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("ecLightDirection", lightDir));
	stateSet->addUniform(new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f)));
	stateSet->setAttributeAndModes(lightModel, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	stateSet->setAttributeAndModes(polygonMode, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	osg::ref_ptr<osgGA::LambdaEventHandler> eventHandler = new osgGA::LambdaEventHandler;
/*
	auto quad = osg::createTexturedQuadGeometry(osg::Vec3(), osg::X_AXIS, osg::Y_AXIS);
	quad->setUseVertexBufferObjects(true);
	geode->addDrawable(quad);
*/
	int faces = osgKaleido::Polyhedron::All;
	int index = 26;

	createPolyhedron(polyhedron, index);
	updatePolyhedron(geode, geometry, polyhedron, faces);

	eventHandler->onKeyDown([&](const osgGA::GUIEventAdapter& ea){
		auto key = ea.getKey();
		auto num = key - osgGA::GUIEventAdapter::KEY_0;
		if (0 <= num && num <= 9)
		{
			faces ^= (1 << wild::mod(num-1, 10));
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