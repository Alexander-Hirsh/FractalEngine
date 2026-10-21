#version 430 core

// Inputs
layout (location = 3) flat in int numCellAround;

// Outputs
layout (location = 0) out vec4 fragColor;

void main()
{
        // vec2 pointCoord = gl_PointCoord * 2.0 - 1.0;
        // if(dot(pointCoord, pointCoord) > 1.0) discard;

        vec4 color0 = vec4(31.0/255.0, 8.0/255.0, 41.0/255.0, 1.0);
        vec4 color1 = vec4(240.0/255.0, 143.0/255.0, 34.0/255.0, 1.0);

        float occlusion = float(numCellAround) / 27.0;
        float shade = 1.0 - 0.985 * occlusion;
        fragColor = vec4(mix(color0, color1, shade));
}
