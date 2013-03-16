#version 400

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;

in vec3 osg_Vertex;
in vec3 osg_Normal;
in vec4 osg_Color; // Kd

uniform vec3 ecLightDirection;
uniform vec3 lightColor; // Ld

out vec4 frontColor;
out vec4 backColor;

vec3 diffuseModel(vec3 lightDirection, vec3 normal, vec3 Ld, vec3 Kd)
{
	return Ld * Kd * max(dot(lightDirection, normal), 0.0);
}

void main()
{
	vec3 ecNormal = normalize(osg_NormalMatrix * osg_Normal);
	
	frontColor = vec4(diffuseModel(ecLightDirection, ecNormal, lightColor, osg_Color.rgb), osg_Color.a);
	backColor = vec4(diffuseModel(ecLightDirection, -ecNormal, lightColor, osg_Color.rgb), osg_Color.a); 
	
	gl_Position = osg_ModelViewProjectionMatrix * vec4(osg_Vertex, 1.0);
}