#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;

in vec3 tePosition[3];
in vec3 tePatchDistance[3];

out vec3 gFacetNormal;
out vec3 gPatchDistance;
out vec3 gTriDistance;
out vec3 gNormal;

void main()
{
    vec3 A = tePosition[2] - tePosition[0];
    vec3 B = tePosition[1] - tePosition[0];
	
    gFacetNormal = osg_NormalMatrix * normalize(cross(A, B));

    gPatchDistance = tePatchDistance[0];
    gTriDistance = vec3(1, 0, 0);
	gNormal = osg_NormalMatrix * normalize(tePosition[0]);
    gl_Position = gl_in[0].gl_Position;
	EmitVertex();

    gPatchDistance = tePatchDistance[1];
    gTriDistance = vec3(0, 1, 0);
	gNormal = osg_NormalMatrix * normalize(tePosition[1]);
    gl_Position = gl_in[1].gl_Position;
	EmitVertex();

    gPatchDistance = tePatchDistance[2];
    gTriDistance = vec3(0, 0, 1);
	gNormal = osg_NormalMatrix * normalize(tePosition[2]);
    gl_Position = gl_in[2].gl_Position;
	EmitVertex();

    EndPrimitive();
}