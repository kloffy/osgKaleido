#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osg/DrawCallbacks>
#include <osg/LightModel>
#include <osg/PolygonMode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/LambdaEventHandler>
#include <osgGA/TrackballManipulator>
#include <osgText/Text>
#include <osgViewer/Viewer>

#pragma warning(pop)

#include <osgKaleido/PolyhedronGeometry>

#include <wild/conversion.hpp>
#include <wild/math.hpp>

osg::Matrixf inverseTranspose(osg::Matrixf const& m)
{
	osg::Matrixf matrix(m);
	matrix.setTrans(0.0, 0.0, 0.0);
	matrix.invert(matrix);
	return osg::Matrixf(
		matrix(0,0), matrix(1,0), matrix(2,0), matrix(3,0),
		matrix(0,1), matrix(1,1), matrix(2,1), matrix(3,1),
		matrix(0,2), matrix(1,2), matrix(2,2), matrix(3,2),
		matrix(0,3), matrix(1,3), matrix(2,3), matrix(3,3)
	);
}

template<typename TArray>
void fill(TArray& items, typename TArray::value_type const& value)
{
	for(auto& item: items) { item = value; }
}

template<typename TArray, typename TMatrix>
void transform(TArray& items, TMatrix const& matrix)
{
	for(auto& item: items) { item = matrix * item; }
}

void makeInstanced(osg::Geometry* geometry, unsigned int numInstances)
{
	if (geometry == nullptr) return;

	osg::Geometry::PrimitiveSetList sets = geometry->getPrimitiveSetList();
	for (auto const& set: sets)
	{
		set->setNumInstances(numInstances);
	}
}

void copy(osg::Uniform* uniform, osg::Vec3Array* data, osg::Vec3 const& defaultValue)
{
	for (auto i = 0u; i < uniform->getNumElements(); ++i)
	{
		if (i < data->size())
		{
			uniform->setElement(i, data->at(i));
		}
		else
		{
			uniform->setElement(i, defaultValue);
		}
	}
	uniform->dirty();
}

class Scene: public osg::Group
{
public:
	static const int VertexInstances = 128;

	Scene(osgViewer::Viewer*, unsigned int, unsigned int);

	void onFaceMaskChanged();
	void onPolyhedronChanged();
	void onLightModelChanged();

	void create();

private:
	void createPolyhedronNode();
	void createVertexNode();
	void createTextNode();

	unsigned int _faces;
	unsigned int _index;
	bool _twoSided;

	osg::observer_ptr<osgViewer::Viewer> _viewer;

	unsigned int _windowWidth;
	unsigned int _windowHeight;

	osg::ref_ptr<osg::Program> _oneSidedProgram;
	osg::ref_ptr<osg::Program> _twoSidedProgram;
	osg::ref_ptr<osg::Program> _instancedProgram;

	osg::ref_ptr<osg::Geode> _pgeode;
	osg::ref_ptr<osg::Geode> _vgeode;

	osg::ref_ptr<osgKaleido::PolyhedronGeometry> _pgeometry;
	osg::ref_ptr<osgKaleido::PolyhedronGeometry> _vgeometry;

	osg::ref_ptr<osgText::Text> _text;
};

Scene::Scene(osgViewer::Viewer* viewer, unsigned int windowWidth, unsigned int windowHeight):
	_viewer(viewer),
	_windowWidth(windowWidth),
	_windowHeight(windowHeight),
	_faces(osgKaleido::PolyhedronGeometry::All),
	_index(26),
	_twoSided(true)
{
	create();
}

void Scene::create()
{
	_oneSidedProgram = new osg::Program;
	_oneSidedProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_directional1_vs.glsl"));
	_oneSidedProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_directional1_fs.glsl"));

	_twoSidedProgram = new osg::Program;
	_twoSidedProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_directional2_vs.glsl"));
	_twoSidedProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_directional2_fs.glsl"));

	_instancedProgram = new osg::Program;
	_instancedProgram->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_billboard_instanced_vs.glsl"));
	_instancedProgram->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_billboard_instanced_fs.glsl"));

	createPolyhedronNode();
	createVertexNode();
	createTextNode();

	onPolyhedronChanged();
	onLightModelChanged();

	osg::ref_ptr<osgGA::LambdaEventHandler> eventHandler = new osgGA::LambdaEventHandler;
	eventHandler->onKeyDown([&](const osgGA::GUIEventAdapter& ea){
		auto key = ea.getKey();
		auto num = key - osgGA::GUIEventAdapter::KEY_0;
		if (0 <= num && num <= 9)
		{
			_faces ^= osgKaleido::PolyhedronGeometry::FaceMaskFromSides(wild::mod(num-1, 10) + 3);
			onFaceMaskChanged();
			return true;
		}
		switch (key)
		{
		case osgGA::GUIEventAdapter::KEY_V:
		{
			_vgeode->setNodeMask(~_vgeode->getNodeMask());
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_L:
		{
			_twoSided = !_twoSided;
			onLightModelChanged();
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Right:
		{
			_faces = osgKaleido::PolyhedronGeometry::All;
			_index++;
			onPolyhedronChanged();
			return true;
		}
		case osgGA::GUIEventAdapter::KEY_Left:
		{
			_faces = osgKaleido::PolyhedronGeometry::All;
			_index--;
			onPolyhedronChanged();
			return true;
		}
		default:
			return false;
		}
	});

	_viewer->addEventHandler(eventHandler);
}

void Scene::createPolyhedronNode()
{
	_pgeode = new osg::Geode;
	_pgeode->setDataVariance(osg::Object::DYNAMIC);

	osg::Vec3f lightDir(0.0f, 0.0f, 1.0f);
	lightDir.normalize();

	auto stateSet = _pgeode->getOrCreateStateSet();
	stateSet->setAttributeAndModes(_twoSidedProgram, osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("ecLightDirection", lightDir));
	stateSet->addUniform(new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f)));

	_pgeometry = new osgKaleido::PolyhedronGeometry("#27");
	_pgeometry->setUseDisplayList(false);
	_pgeometry->setUseVertexBufferObjects(true);

	_pgeode->addDrawable(_pgeometry);

	addChild(_pgeode);
}

void Scene::createVertexNode()
{
	_vgeode = new osg::Geode;
	_vgeode->setDataVariance(osg::Object::DYNAMIC);

	osg::Vec3f lightDir(1.0f, 1.0f, 1.0f);
	lightDir.normalize();

	auto offsets = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "offsets", Scene::VertexInstances);
	offsets->setDataVariance(osg::Object::DYNAMIC);

	auto stateSet = _vgeode->getOrCreateStateSet();
	stateSet->setAttributeAndModes(_instancedProgram, osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("ecLightDirection", lightDir));
	stateSet->addUniform(new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f)));
	stateSet->addUniform(offsets);

	osg::Vec3 scale(1.0f, 1.0f, 1.0f);
	osg::Vec4 color(0.25f, 0.25f, 0.25f, 1.0f);

	auto vertexMatrix = osg::Matrixf::scale(scale * 0.125f * 0.25f);
	auto normalMatrix = inverseTranspose(vertexMatrix);

	_vgeometry = new osgKaleido::PolyhedronGeometry("#27");
	_vgeometry->setUseDisplayList(false);
	_vgeometry->setUseVertexBufferObjects(true);

	_vgeometry->update(nullptr); // Force geometry generation

	auto vertices = dynamic_cast<osg::Vec3Array*>(_vgeometry->getVertexArray());
	if (vertices != nullptr) { transform(*vertices, vertexMatrix); }

	auto normals = dynamic_cast<osg::Vec3Array*>(_vgeometry->getNormalArray());
	if (normals != nullptr) { transform(*normals, normalMatrix); }

	auto colors = dynamic_cast<osg::Vec4Array*>(_vgeometry->getColorArray());
	if (colors != nullptr) { fill(*colors, color); }

	makeInstanced(_vgeometry, Scene::VertexInstances);

	auto size = 1.0f;
	osg::BoundingBox bb(-size, -size, -size, +size, +size, +size);
	_vgeometry->setInitialBound(bb);

	_vgeode->addDrawable(_vgeometry);

	addChild(_vgeode);
}

void Scene::createTextNode()
{
	osg::ref_ptr<osg::Geode> tgeode = new osg::Geode;
	tgeode->setDataVariance(osg::Object::DYNAMIC);

	osg::Projection* projection = new osg::Projection;
	projection->setMatrix(osg::Matrix::ortho2D(0, _windowWidth, 0, _windowHeight));

	osg::MatrixTransform* modelView = new osg::MatrixTransform;
	modelView->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	modelView->setMatrix(osg::Matrix::identity());

	_text = new osgText::Text();
	_text->setAxisAlignment(osgText::Text::SCREEN);
	_text->setAlignment(osgText::Text::CENTER_TOP);
	_text->setFont("/fonts/arial.ttf");
	_text->setCharacterSize(16.0f);
	_text->setLineSpacing(0.25f);
	_text->setPosition(osg::Vec3(_windowWidth/2.0f, _windowHeight - 10.0f, -1.5f));

	osg::ref_ptr<osg::UseVertexAttributeAliasing> vaa = new osg::UseVertexAttributeAliasing(false);
	_text->setDrawCallback(vaa);

	auto stateSet = tgeode->getOrCreateStateSet();
	stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateSet->setRenderBinDetails(15, "DepthSortedBin", osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

	tgeode->addDrawable(_text);
	modelView->addChild(tgeode);
	projection->addChild(modelView);

	addChild(projection);
}

void Scene::onFaceMaskChanged()
{
	_pgeometry->setFaceMask(static_cast<osgKaleido::PolyhedronGeometry::FaceMask>(_faces));
}

void Scene::onPolyhedronChanged()
{
	// Update Polyhedron
	auto symbol = "#" + wild::conversion_cast<std::string>(wild::mod(_index, 80) + 1);

	_pgeometry->setSymbol(symbol);
	_pgeometry->setFaceMask(static_cast<osgKaleido::PolyhedronGeometry::FaceMask>(_faces));

	auto polyhedron = _pgeometry->getOrCreatePolyhedron();

	OSG_INFO << polyhedron->getName() << " (" << polyhedron->getDualName() << "*)" << std::endl;
	OSG_INFO << polyhedron->getWythoffSymbol() << std::endl;
	OSG_INFO << polyhedron->getVertexConfiguration() << std::endl;
	OSG_INFO << polyhedron->getVertexCount() << std::endl;
	OSG_INFO << polyhedron->getFaceCount() << std::endl;

	// Update Vertices
	osg::ref_ptr<osg::Vec3Array> vertices = osgKaleido::createVertexArray(*polyhedron);

	auto stateSet = _vgeode->getOrCreateStateSet();
	auto offsets = stateSet->getUniform("offsets");
	
	copy(offsets, vertices, osg::Vec3());

	// Update Text
	_text->setText(polyhedron->getName() + "\n" + polyhedron->getWythoffSymbol());
}

void Scene::onLightModelChanged()
{
	auto program = _twoSided ? _twoSidedProgram : _oneSidedProgram;

	auto stateSet = _pgeode->getOrCreateStateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
}

int main(int argc, char** argv)
{
/*
	osg::ref_ptr<osg::Geode> test = new osg::Geode;

	osg::ref_ptr<osgKaleido::PolyhedronGeometry> testGeometry = new osgKaleido::PolyhedronGeometry("#3", osgKaleido::PolyhedronGeometry::Triangular);
	test->addDrawable(testGeometry);

	std::ostringstream sstream;
	osg::ref_ptr<osgDB::Options> options = new osgDB::Options("Ascii");
	osgDB::ReaderWriter::WriteResult wr = osgDB::Registry::instance()->getReaderWriterForExtension("osgt")->writeObject(*test, sstream, options);
	std::cout << sstream.str() << std::endl;
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

	osgViewer::Viewer viewer(arguments);
	viewer.setUpViewInWindow((screenSettings.width - windowWidth)/2, (screenSettings.height - windowHeight)/2, 800, 600);
	viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
	viewer.setRunMaxFrameRate(0.0);
	viewer.realize();

	osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);

	for (auto& window: windows)
	{
		window->setWindowName("osgKaleidoViewer");
		auto state = window->getState();
		state->setUseModelViewAndProjectionUniforms(true);
		state->setUseVertexAttributeAliasing(true);
	}

	osg::ref_ptr<Scene> scene = new Scene(&viewer, windowWidth, windowHeight);

	viewer.setSceneData(scene);

	return viewer.run();
}