#version 400

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;

in vec3 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_MultiTexCoord0;

out vec2 texCoord;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * vec4(osg_Vertex, 1.0);

	texCoord = osg_MultiTexCoord0.st;
}