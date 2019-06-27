#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0); //for 2D projection we only need the XY coords
    TexCoords = vertex.zw; //the texture coords are placed in the last two values of the input vector
}  