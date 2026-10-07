#version 430 core

uniform mat4 cameraSpaceTransform;

layout(std430, binding = 0) readonly buffer PointBuffer
{
        vec4 points[];
};

void main()
{
        vec4 point = points[gl_VertexID];
        gl_Position = cameraSpaceTransform * point;
        gl_PointSize = 1.0;
}
