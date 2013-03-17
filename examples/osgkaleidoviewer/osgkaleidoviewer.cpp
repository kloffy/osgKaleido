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
	OSG_INFO << polyhedron->getVertexCount() << std::endl;
	OSG_INFO << polyhedron->getFaceCount() << std::endl;
}

osg::Uniform* createUniformArray(std::string const& name, unsigned int numElements, osg::Vec3Array* data, osg::Vec3 const& _default)
{
	osg::Uniform* result = new osg::Uniform(osg::Uniform::FLOAT_VEC3, name, numElements);
	for (auto i = 0u; i < numElements; ++i)
	{
		if (i < data->size())
		{
			result->setElement(i, data->at(i));
		}
		else
		{
			result->setElement(i, _default);
		}
	}
	return result;
}

void makeInstanced(osg::ref_ptr<osg::Geometry>& geometry, int nInstances)
{
	osg::Geometry::PrimitiveSetList sets = geometry->getPrimitiveSetList();
	for (auto const& set: sets)
	{
		set->setNumInstances(nInstances);
	}
}

osg::Geode* createVertexGeode()
{
	osg::ref_ptr<osg::Geode> result = new osg::Geode;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/instanced_vs.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/instanced_fs.glsl"));

	auto stateSet = result->getOrCreateStateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);

	return result.release();
}

osg::Geode* createFaceGeode()
{
	osg::ref_ptr<osg::Geode> result = new osg::Geode;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_directional2_vs.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_directional2_fs.glsl"));

	osg::Vec3f lightDir(0.0f, 0.0f, 1.0f);
    lightDir.normalize();

	auto stateSet = result->getOrCreateStateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("ecLightDirection", lightDir));
	stateSet->addUniform(new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f)));

	return result.release();
}

void updateVertexGeode(osg::ref_ptr<osg::Geode>& geode, osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, osg::ref_ptr<osg::Geometry>& geometry)
{
	osg::ref_ptr<osg::Vec3Array> vertices = osgKaleido::createVertices(polyhedron);

	OSG_WARN << vertices->size() << std::endl;

	auto instances = 128;
	auto offsets = createUniformArray("offsets", instances, vertices, osg::Vec3());
	
	auto stateSet = geode->getOrCreateStateSet();
	stateSet->removeUniform("offsets");
	stateSet->addUniform(offsets);
	
	makeInstanced(geometry, instances);

	auto size = 1.0f;
	osg::BoundingBox bb(-size, -size, -size, +size, +size, +size);
	geometry->setInitialBound(bb);

	geode->addDrawable(geometry);
}

void updateFaceGeode(osg::ref_ptr<osg::Geode>& geode, osg::ref_ptr<osgKaleido::Polyhedron>& polyhedron, osg::ref_ptr<osg::Geometry>& geometry, int faces)
{
	if (geometry) geode->removeDrawable(geometry);
	geometry = osgKaleido::createFaces(polyhedron.get(), static_cast<osgKaleido::Polyhedron::Faces>(faces));
	geometry->setUseVertexBufferObjects(true);
	if (geometry) geode->addDrawable(geometry);
}

osg::Matrix3 inverseTranspose(osg::Matrixf const& m)
{
	osg::Matrixf matrix(m);
	matrix.setTrans(0.0, 0.0, 0.0);
	matrix.invert(matrix);
    return osg::Matrix3(
		matrix(0,0), matrix(1,0), matrix(2,0),
        matrix(0,1), matrix(1,1), matrix(2,1),
        matrix(0,2), matrix(1,2), matrix(2,2)
	);
}

void transform(osg::ref_ptr<osg::Geometry>& geometry, osg::Matrixf const& matrix)
{
	auto vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
	if (vertices != nullptr)
	{
		for(auto& vertex: (*vertices)) vertex = matrix * vertex;
	}
	auto normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
	if (normals != nullptr)
	{
		auto normalMatrix = inverseTranspose(matrix);
		for(auto& normal: (*normals)) normal = matrix * normal;
	}
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

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> vgeode = createVertexGeode();
	osg::ref_ptr<osg::Geode> fgeode = createFaceGeode();

	osg::ref_ptr<osgKaleido::Polyhedron> polyhedron;
	osg::ref_ptr<osg::Geometry> vgeometry;
	osg::ref_ptr<osg::Geometry> fgeometry;


	osgViewer::Viewer viewer(arguments);
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	//viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
	//viewer.setRunFrameScheme(osgViewer::Viewer::CONTINUOUS);
	//viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	//osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
	//lightModel->setTwoSided(true);

	//osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode;
	//polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
/*
	osg::Vec2 extent(0.125f, 0.125f);
	osg::Vec3 corner(-extent.x()/2.0f,-extent.y()/2.0f, 0.0f);
	vgeometry = osg::createTexturedQuadGeometry(corner, osg::X_AXIS * extent.x(), osg::Y_AXIS * extent.y());
	vgeometry->setUseVertexBufferObjects(true);
*/
	auto state = viewer.getCamera()->getGraphicsContext()->getState();
	state->setUseModelViewAndProjectionUniforms(true);
	state->setUseVertexAttributeAliasing(true);
	
	osg::ref_ptr<osgGA::LambdaEventHandler> eventHandler = new osgGA::LambdaEventHandler;
	
	int faces = osgKaleido::Polyhedron::All;
	int index = 26;

	createPolyhedron(polyhedron, index);
	vgeometry = osgKaleido::createFaces(polyhedron);
	osg::Vec3f v(1.0f, 1.0f, 1.0f);
	osg::Matrixf m = osg::Matrixf::scale(v * 0.125f * 0.25f);
	transform(vgeometry, m);
	vgeometry->setUseVertexBufferObjects(true);

	updateVertexGeode(vgeode, polyhedron, vgeometry);
	updateFaceGeode(fgeode, polyhedron, fgeometry, faces);

	eventHandler->onKeyDown([&](const osgGA::GUIEventAdapter& ea){
		auto key = ea.getKey();
		auto num = key - osgGA::GUIEventAdapter::KEY_0;
		if (0 <= num && num <= 9)
		{
			faces ^= osgKaleido::Polyhedron::sidesToFace(wild::mod(num-1, 10) + 3);
			updateFaceGeode(fgeode, polyhedron, fgeometry, faces);
			return true;
		}
		switch (key)
		{
		case osgGA::GUIEventAdapter::KEY_V:
		{
			vgeode->setNodeMask(~vgeode->getNodeMask());
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_L:
		{
			//lightModel->setTwoSided(!lightModel->getTwoSided());
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Right:
		{
			index++;
			faces = osgKaleido::Polyhedron::All;
			createPolyhedron(polyhedron, index);
			updateVertexGeode(vgeode, polyhedron, vgeometry);
			updateFaceGeode(fgeode, polyhedron, fgeometry, faces);
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Left:
		{
			index--;
			faces = osgKaleido::Polyhedron::All;
			createPolyhedron(polyhedron, index);
			updateVertexGeode(vgeode, polyhedron, vgeometry);
			updateFaceGeode(fgeode, polyhedron, fgeometry, faces);
			return true;
		}
		default:
			return false;
		}
	});

	root->addChild(vgeode);
	root->addChild(fgeode);

	viewer.setSceneData(root.get());
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);
	viewer.addEventHandler(eventHandler);

	return viewer.run();
}