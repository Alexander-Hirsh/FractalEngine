#version 430 core

uniform mat4 perspectProjection;
uniform vec2 cameraPosition;

layout(std430, binding = 0) readonly buffer PointBuffer
{
        vec4 points[];
};

void main()
{
        vec4 point = points[gl_VertexID];
        vec3 viewPosition = vec3(point.xy - cameraPosition, point.z);
        gl_Position = perspectProjection * vec4(viewPosition, 1.0);
        gl_PointSize = 1.0;
}
