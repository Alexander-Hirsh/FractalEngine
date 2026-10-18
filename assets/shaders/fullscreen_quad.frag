#version 430 core

out vec4 fragColor;
in vec2 texCoords;

uniform sampler2D screenTex;

void main()
{
    fragColor = texture(screenTex, texCoords);
}