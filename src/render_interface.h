#pragma once

#include <string>

#include "assets.h"
#include "math_lib.h"


/// ##############################################################################################
///                                     Render Constants
/// ##############################################################################################
constexpr uint64_t MAX_POINTS = 1000000;
constexpr int POINT_COMPUTE_LOCAL_SIZE = 256;
constexpr uint64_t MAX_COMPUTE_WORK_GROUP_COUNT_X = 65535;
constexpr int AO_GRID_SIDE_SIZE = 32;

constexpr int RENDER_WIDTH = 1920/1.5;
constexpr int RENDER_HEIGHT = 1080/1.5;
static_assert(MAX_POINTS <= POINT_COMPUTE_LOCAL_SIZE * MAX_COMPUTE_WORK_GROUP_COUNT_X * MAX_COMPUTE_WORK_GROUP_COUNT_X ,
              "MAX_POINTS must fit in one compute dispatch");

const char* POINT_PROGRAM_NAME = "point_cloud";
const char* VOXEL_PROGRAM_NAME = "voxel_cube";


/// ##############################################################################################
///                                     Render Structs
/// ##############################################################################################
struct Camera3D
{
        float zoom = 1.0f;
        vec3 position = vec3(0.5, 0.5, 4.0);
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
///                                     Simulation Structs
/// ##############################################################################################
struct IFS
{
        int currNumOfMatrix = 0;
        static const int maxNumOfMatrix = 4;

        vec3 currScale[maxNumOfMatrix] = {};
        vec3 currRotation[maxNumOfMatrix] = {};
        vec3 currShear[maxNumOfMatrix] = {};
        vec3 currTranslation[maxNumOfMatrix] = {};

        vec3 newScale[maxNumOfMatrix] = {};
        vec3 newRotation[maxNumOfMatrix] = {};
        vec3 newShear[maxNumOfMatrix] = {};
        vec3 newTranslation[maxNumOfMatrix] = {};

        mat4 IFSMatrices[maxNumOfMatrix] = {};

        float iCurve = 0.9975f;

        void generateNewParameters()
        {
                currNumOfMatrix = (int)(rng_f() * (maxNumOfMatrix - 1)) + 2;

                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        newScale[i] = randVec3(0.65f, 0.75f);
                        newRotation[i] = randVec3(-degToRad(40), degToRad(40));
                        newShear[i] = randVec3(-0.1f, 0.1f);
                        newTranslation[i] = randVec3(0.0f, 1.0f);
                }
        }

        void generateTransformMatrix(float dt)
        {
                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        vec3Lerp(currScale[i], newScale[i], iCurve, dt);
                        vec3Lerp(currRotation[i], newRotation[i], iCurve, dt);
                        vec3Lerp(currShear[i], newShear[i], iCurve, dt);
                        vec3Lerp(currTranslation[i], newTranslation[i], iCurve, dt);

                        IFSMatrices[i] = translationMatrix(currTranslation[i]) *
                                         rotationMatrix(currRotation[i])    *
                                         shearMatrix(currShear[i])          *
                                         scaleMatrix(currScale[i]);
                }
        }
};

/// ##############################################################################################
///                                     Render Globals
/// ##############################################################################################
static RenderData* renderData;


/// ##############################################################################################
///                                     Render Functions
/// ##############################################################################################