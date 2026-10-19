#version 430 core

uniform sampler2D srcTexture;
uniform vec2 filterRadius;

in vec2 texCoords;
layout (location = 0) out vec3 upsample;

void main()
{
    float x = filterRadius.x;
    float y = filterRadius.y;

    vec3 a = texture(srcTexture, vec2(texCoords.x - x, texCoords.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(texCoords.x,     texCoords.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(texCoords.x + x, texCoords.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(texCoords.x - x, texCoords.y)).rgb;
    vec3 e = texture(srcTexture, texCoords).rgb;
    vec3 f = texture(srcTexture, vec2(texCoords.x + x, texCoords.y)).rgb;

    vec3 g = texture(srcTexture, vec2(texCoords.x - x, texCoords.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(texCoords.x,     texCoords.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(texCoords.x + x, texCoords.y - y)).rgb;

    upsample = (e * 4.0 + (b + d + f + h) * 2.0 + a + c + g + i) / 16.0;
}
