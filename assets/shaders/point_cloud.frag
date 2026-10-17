#version 430 core

// Inputs
layout (location = 3) flat in int numCellAround;

// Outputs
layout (location = 0) out vec4 fragColor;

void main()
{
        // vec2 pointCoord = gl_PointCoord * 2.0 - 1.0;
        // if(dot(pointCoord, pointCoord) > 1.0) discard;

        float occlusion = float(numCellAround) / 27.0;
        float shade = 1.0 - 0.8 * occlusion;
        fragColor = vec4(vec3(shade), 1.0);
}
