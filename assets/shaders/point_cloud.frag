#version 430 core

// Inputs
layout (location = 3) flat in int numCellAround;

// Outputs
layout (location = 0) out vec4 fragColor;

void main()
{
        // vec2 pointCoord = gl_PointCoord * 2.0 - 1.0;
        // if(dot(pointCoord, pointCoord) > 1.0) discard;

        float shade = numCellAround / 27; 
        fragColor = vec4(1,1,1,1) * shade + 0.1;
}
