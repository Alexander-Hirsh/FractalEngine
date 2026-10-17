#version 430 core

// Buffers in
layout(std430, binding = 0) readonly buffer PointBuffer
{
        vec4 points[];
};

layout(std430, binding = 2) readonly buffer AOGridBuffer
{
        uint AOgrid[];
};

// Uniforms
uniform mat4 cameraSpaceTransform;
uniform int AOgridSize;

// Outputs
layout (location = 3) flat out int numCellAround;

// Functions
void main()
{
        vec4 point = points[gl_VertexID];

        ivec3 gridCell = ivec3(floor((point.xyz + vec3(0.5)) * float(AOgridSize)));
        int numCellOccupied = 0;
        if(all(greaterThanEqual(gridCell, ivec3(0))) &&
           all(lessThan(gridCell, ivec3(AOgridSize))))
        {
                for(int x = -1; x <= 1; x++)
                {
                        for(int y = -1; y <= 1; y++)
                        {
                                for(int z = -1; z <= 1; z++)
                                {
                                        ivec3 neighbor = gridCell + ivec3(x, y, z);
                                        if(any(lessThan(neighbor, ivec3(0))) ||
                                           any(greaterThanEqual(neighbor, ivec3(AOgridSize))))
                                                continue;

                                        uint cellIndex = uint(neighbor.x +
                                                AOgridSize * (neighbor.y + AOgridSize * neighbor.z));
                                        if(AOgrid[cellIndex] != 0u)
                                                numCellOccupied++;
                                }
                        }                
                }
        }

        numCellAround = numCellOccupied;
        gl_Position = cameraSpaceTransform * point;
        gl_PointSize = 1.0;
}
