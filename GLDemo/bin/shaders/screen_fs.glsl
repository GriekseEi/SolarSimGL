#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

//draws the texture of the view space to the screen
void main() {
	vec3 col = texture(screenTexture, TexCoords).rgb;
	FragColor = vec4(col, 1.0);

	//potential post-processing effects can be implemented here
}