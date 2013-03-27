#pragma warning(push)
#pragma warning(disable: 4250)

#include <osg/ArgumentParser>
#include <osg/LightModel>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgUtil/SmoothingVisitor>
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

	osgViewer::Viewer::Windows windows;
	viewer.getWindows(windows);

	for (auto& window: windows)
	{
		auto state = window->getState();
		state->setUseModelViewAndProjectionUniforms(true);
		state->setUseVertexAttributeAliasing(true);
	}

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, "data/diffuse_sphere_vs.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::TESSCONTROL, "data/diffuse_sphere_tc.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::TESSEVALUATION, "data/diffuse_sphere_te.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, "data/diffuse_sphere_gs.glsl"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, "data/diffuse_sphere_fs.glsl"));

	program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, 3);
	program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
	program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
	program->setParameter(GL_PATCH_VERTICES, 3);

	osg::ref_ptr<osg::Geode> root = new osg::Geode;

	osg::Vec3f lightDir(0.0f, 0.0f, 1.0f);
	lightDir.normalize();

	osg::ref_ptr<osg::Uniform> tessInner = new osg::Uniform("tessLevelInner", 5.0f);
	osg::ref_ptr<osg::Uniform> tessOuter = new osg::Uniform("tessLevelOuter", 5.0f);

	auto stateSet = root->getOrCreateStateSet();
	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
	stateSet->addUniform(tessInner);
	stateSet->addUniform(tessOuter);
	stateSet->addUniform(new osg::Uniform("ecLightDirection", lightDir));
	stateSet->addUniform(new osg::Uniform("lightColor", osg::Vec3(1.0f, 1.0f, 1.0f)));
	
	osg::ref_ptr<osgKaleido::Polyhedron> icosahedron = new osgKaleido::Polyhedron("#27");

	osg::ref_ptr<osg::Geometry> geometry = osgKaleido::createGeometry(*icosahedron);
	geometry->setUseDisplayList(false);
	geometry->setUseVertexBufferObjects(true);

	auto size = 1.0f;
	osg::BoundingBox bb(-size, -size, -size, +size, +size, +size);
	geometry->setInitialBound(bb);

	for(auto& primitiveSet: geometry->getPrimitiveSetList())
	{
		// Icosahedron only has triangle faces, so this is safe
		//assert(primitiveSet->getMode() == osg::PrimitiveSet::POLYGON);
		primitiveSet->setMode(osg::PrimitiveSet::PATCHES);
	}

	root->addDrawable(geometry);

	viewer.setSceneData(root.get());

	return viewer.run();
}