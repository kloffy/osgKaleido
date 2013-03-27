#version 400

in vec2 texCoord;

out vec4 fragData;

void main()
{
	fragData = vec4(texCoord.s, texCoord.t, 0.0, 1.0);
}