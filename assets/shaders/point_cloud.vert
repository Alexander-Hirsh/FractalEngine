#version 430 core

// Buffers in
layout(std430, binding = 0) readonly buffer PointBuffer
{
        vec4 points[];
};

layout(std430, binding = 2) buffer AOGridBuffer
{
        bool AOgrid[];
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

        int gridX = int((point.x + 0.5) * AOgridSize);
        int gridY = int((point.y + 0.5) * AOgridSize);
        int gridZ = int((point.z + 0.5) * AOgridSize);

        int numCellOccupied = 0;
        for(int x = -1; x <= 1; x++)
        {
                for(int y = -1; y <= 1; y++)
                {
                        for(int z = -1; z <= 1; z++)
                        {
                                int pX = x + gridX;
                                int pY = y + gridY;
                                int pZ = z + gridZ;

                                if(pX < 0 || pX > AOgridSize || 
                                   pY < 0 || pY > AOgridSize ||
                                   pZ < 0 || pZ > AOgridSize)
                                        continue;

                                if(AOgrid[(pX) + AOgridSize * ((pY) + AOgridSize * (pZ))]) 
                                        numCellOccupied++;
                        }                
                }       
        }

        numCellAround = numCellOccupied;
        gl_Position = cameraSpaceTransform * point;
        gl_PointSize = 1.0;
}
