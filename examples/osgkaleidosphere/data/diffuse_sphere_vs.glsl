#version 400

// Based on: http://prideout.net/blog/?p=48

in vec3 osg_Vertex;

out vec3 vPosition;

void main()
{
	vPosition = osg_Vertex;
}