#version 400

layout(triangles, equal_spacing, cw) in;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;

in vec3 tcPosition[];

out vec3 tePosition;
out vec3 tePatchDistance;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    tePatchDistance = gl_TessCoord;
    tePosition = normalize(p0 + p1 + p2);
    gl_Position = osg_ModelViewProjectionMatrix * vec4(tePosition, 1);
}