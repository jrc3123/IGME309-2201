#version 330

in vec3 Color;

vec3 ComplementColor = vec3(1.0, 1.0, 1.0) - Color;
uniform vec3 SolidColor = vec3(-1,-1,-1);
out vec4 Fragment;
uniform bool pressed;
void main()
{
	if(pressed)
	Fragment = vec4(ComplementColor,1);
	else
	Fragment = vec4(Color, 1);
	
	if(SolidColor.r != -1.0 && SolidColor.g != -1.0 && SolidColor.b != -1.0)
		Fragment = vec4(SolidColor, 1);

	return;
}