#version 400

// Based on: http://www.gamedev.net/page/resources/_/technical/opengl/the-basics-of-glsl-40-shaders-r2861

const int NUM_INSTANCES = 128;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform mat3 osg_NormalMatrix;

uniform vec3 offsets[NUM_INSTANCES];

in vec3 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_Color; // Kd

uniform vec3 ecLightDirection;
uniform vec3 lightColor; // Ld

out vec4 frontColor;

vec3 diffuseModel(vec3 lightDirection, vec3 normal, vec3 Ld, vec3 Kd)
{
	return Ld * Kd * max(dot(lightDirection, normal), 0.0);
}

void main()
{
	vec3 ecNormal = normalize(osg_Normal);
	
	frontColor = vec4(diffuseModel(ecLightDirection, ecNormal, lightColor, osg_Color.rgb), osg_Color.a);
	
	//gl_Position = osg_ModelViewProjectionMatrix * vec4(osg_Vertex + offset, 1.0);
	
	vec3 offset = offsets[gl_InstanceID];
	gl_Position = osg_ProjectionMatrix * (osg_ModelViewMatrix * vec4(offset, 1.0) + vec4(osg_Vertex, 0.0));
}