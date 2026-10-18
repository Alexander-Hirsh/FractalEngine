#version 430 core

out vec2 texCoords;

vec2 positions[4] = 
{
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
};

vec2 texCoordsArray[4] = 
{
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
};

void main()
{
        gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
        texCoords = texCoordsArray[gl_VertexID];
}