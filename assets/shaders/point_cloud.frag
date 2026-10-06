#version 430 core

layout (location = 0) out vec4 fragColor;

void main()
{
        // vec2 pointCoord = gl_PointCoord * 2.0 - 1.0;
        // if(dot(pointCoord, pointCoord) > 1.0) discard;
        fragColor = vec4(1,1,1,1);
}
