#version 400

in vec4 frontColor;
in vec4 backColor;

out vec4 fragData;

void main() 
{
	if(gl_FrontFacing)
	{
		fragData = frontColor;
	}
	else
	{
		fragData = backColor;
	}
}