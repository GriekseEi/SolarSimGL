#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0); //perspective division
	
	/*
	Because we want the skybox to pass the depth test where possible even though it's drawn later than the planetoids, we want to keep the z-value
	(whose value is the resulting depth value after perspective division has been performed and then divided by w) to always be 1.0, so it's set to have the same
	value as w, whose value is always 1.0, so by assigning the value of w to z we end up with w / w = 1.0.
	*/
    gl_Position = pos.xyww;
}  