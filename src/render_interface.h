#pragma once

#include "assets.h"
#include "math_lib.h"


/// ##############################################################################################
///                                     Render Constants
/// ##############################################################################################
constexpr int MAX_POINTS = 65535;
constexpr int POINT_COMPUTE_LOCAL_SIZE = 256;
constexpr int MAX_COMPUTE_WORK_GROUP_COUNT_X = 65535;
constexpr int RENDER_WIDTH = 1280;
constexpr int RENDER_HEIGHT = 720;
static_assert(MAX_POINTS <= POINT_COMPUTE_LOCAL_SIZE * MAX_COMPUTE_WORK_GROUP_COUNT_X,
              "MAX_POINTS must fit in one compute dispatch");


/// ##############################################################################################
///                                     Render Structs
/// ##############################################################################################
struct Camera3D
{
        float zoom = 1.0f;
        vec3 position;
        float fov = 90.0f;

        float nearPlane = 0.1f;
        float farPlane = 100.0f;
};

struct RenderData
{
        Camera3D gameCamera;
        int pointCount;
};


/// ##############################################################################################
///                                     Render Globals
/// ##############################################################################################
static RenderData* renderData;


/// ##############################################################################################
///                                     Render Functions
/// ##############################################################################################