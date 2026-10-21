#version 430 core

// Buffers in
layout(std430, binding = 0) readonly buffer PointBuffer { vec4 points[]; };
layout(std430, binding = 4) readonly buffer ReductionBuffer { uint reducedGrid[]; };
layout(std430, binding = 1) readonly buffer TransformBuffer { mat4 transforms[]; };

// Uniforms
uniform mat4 cameraSpaceTransform;
uniform int AOgridSize;

// Outputs
layout (location = 3) flat out int numCellAround;

uint cellIndex(ivec3 p) {
        return uint(p.x + AOgridSize * (p.y + AOgridSize * p.z));
}

// Functions
void main()
{
        vec4 point = points[gl_VertexID];

        ivec3 gridCell = ivec3(floor((point.xyz + vec3(0.5)) * float(AOgridSize)));

        numCellAround = 0;
        if(all(greaterThanEqual(gridCell, ivec3(0))) &&
           all(lessThan(gridCell, ivec3(AOgridSize))))
        {
                numCellAround = int(reducedGrid[cellIndex(gridCell)]);
        }

        gl_Position = cameraSpaceTransform * point;
        gl_PointSize = 1.0;
}
