#version 400

in vec4 frontColor;

out vec4 fragData;

void main() 
{
	fragData = frontColor;
}