#pragma once

#include <string>
#include <vector>

#include "assets.h"
#include "math_lib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/// ##############################################################################################
///                                     Render Constants
/// ##############################################################################################
constexpr uint64_t MAX_POINTS = 4000000;
constexpr int BOUNDS_DEBUG_VERTEX_COUNT = 24;
constexpr int POINT_COMPUTE_LOCAL_SIZE = 256;
constexpr uint64_t MAX_COMPUTE_WORK_GROUP_COUNT_X = 65535;
constexpr int AO_GRID_SIDE_SIZE = 256;
constexpr uint64_t AO_GRID_CELL_COUNT = uint64_t(AO_GRID_SIDE_SIZE) * AO_GRID_SIDE_SIZE * AO_GRID_SIDE_SIZE;
constexpr uint64_t AO_GRID_32BIT_GRID_SIZE = (AO_GRID_CELL_COUNT + 31) / 32;
constexpr uint64_t REDUCTED_AO_GRID_32BIT = (AO_GRID_CELL_COUNT + 7) / 8;

constexpr int RENDER_WIDTH = 1920;
constexpr int RENDER_HEIGHT = 1080;
static_assert(MAX_POINTS <= POINT_COMPUTE_LOCAL_SIZE * MAX_COMPUTE_WORK_GROUP_COUNT_X * MAX_COMPUTE_WORK_GROUP_COUNT_X ,
              "MAX_POINTS must fit in one compute dispatch");

constexpr unsigned int MIP_CHAIN_LENGTH = 4;
constexpr int MIP_RATIO = 3;

const char* POINT_PROGRAM_NAME = "point_cloud";
const char* IFS_PROGRAM_NAME = "IFS";
const char* BOUND_PROGRAM_NAME = "bounds_compute";
const char* FULLSCREEN_PROGRAM_NAME = "fullscreen_quad";
const char* DOWNSCALE_PROGRAM_NAME = "downscale";
const char* UPSCALE_PROGRAM_NAME = "upscale";
const char* REDUCTION_PROGRAM_NAME = "reduction";


/// ##############################################################################################
///                                     Render Structs
/// ##############################################################################################
struct Camera3D
{
        float zoom = 1.0f;
        vec3 position;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float distance = 1.0f;

        vec3 fpoint = vec3(0.0, 0.0, 0.0);
        vec3 forward, right, up;

        float fov = 90.0f;

        float nearPlane = 0.1f;
        float farPlane = 100.0f;

        void updateCamera(const Input& input)
        {
                if(input.keys[KEY_MOUSE_RIGHT].isDown)
                {
                        constexpr float mouseSensitivity = 0.005f;
                        float maxPitch = degToRad(89.0f);
                        yaw += input.relMousePos.x * mouseSensitivity;
                        pitch -= input.relMousePos.y * mouseSensitivity;
                        if(pitch > maxPitch) pitch = maxPitch;
                        if(pitch < -maxPitch) pitch = -maxPitch;
                }

                constexpr float zoomPerNotch = 0.1f;
                float wheelNotches = (float)input.mouseWheelDelta / 120.0f;
                distance -= wheelNotches * zoomPerNotch;
                if(distance < 0.1f) distance = 0.1f;
                if(distance > 100.0f) distance = 100.0f;

                float horizontalDistance = distance * cosf(pitch);
                position = fpoint + vec3(horizontalDistance * sinf(yaw),
                                         distance * sinf(pitch),
                                         horizontalDistance * cosf(yaw));
                forward = normalize(fpoint - position);
                right = normalize(cross(forward, vec3(0.0f, 1.0f, 0.0f)));
                up = cross(right, forward);
        }
};

struct RenderData
{
        Camera3D gameCamera;
        int pointCount;
};

struct bloomMip
{
        ivec2 iSize;
        GLuint texture;
};

/// ##############################################################################################
///                                     Simulation Structs
/// ##############################################################################################
struct PARAMETERS
{
        vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
        vec4 shadowColor = vec4(0.0, 0.0, 0.0, 1.0);
        float shadowThreshold = 0.985;

        vec2 scaleBounds = vec2(0.45f, 0.75f);
        vec2 rotationBounds = vec2(-35.0f, 35.0f);
        vec2 shearBounds = vec2(-0.15f, 0.15f);
        vec2 translationBounds = vec2(0.0f, 3.0f);

        bool useKuwahara = false;
        bool useBloom = false;
        float bloomStrength = 0.25f;
        float gamma = 1.1f;

        float timeToChange = 3.0f;
        float iCurve = 0.5995f;

        bool pause = false;

        int dispatches = 1;
        int screenshotDispatches = 500;
        int AOgridSize = 256;

        bool hideGUI = false;
};

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

        float iCurve = 0.5995f;

        void generateNewParameters(PARAMETERS params)
        {
                currNumOfMatrix = maxNumOfMatrix;//(int)(rng_f() * (maxNumOfMatrix - 1)) + 3;
                iCurve = params.iCurve;

                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        newScale[i] = randVec3(params.scaleBounds.x, params.scaleBounds.y);
                        newRotation[i] = randVec3(degToRad(params.rotationBounds.x), degToRad(params.rotationBounds.y));
                        newShear[i] = randVec3(params.shearBounds.x, params.shearBounds.y);
                        newTranslation[i] = randVec3(params.translationBounds.x, params.translationBounds.y);
                }
        }

        // newScale[i] = randVec3(0.45f, 0.75f);
        // newRotation[i] = randVec3(degToRad(-35), degToRad(35));
        // newShear[i] = randVec3(-0.15f, 0.15f);
        // newTranslation[i] = randVec3(0.0f, 3.0f);

        void generateTransformMatrix(bool pause, float dt)
        {
                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        if(!pause)
                        {
                                vec3Lerp(currScale[i], newScale[i], iCurve, dt);
                                vec3Lerp(currRotation[i], newRotation[i], iCurve, dt);
                                vec3Lerp(currShear[i], newShear[i], iCurve, dt);
                                vec3Lerp(currTranslation[i], newTranslation[i], iCurve, dt);
                        }

                        IFSMatrices[i] = translationMatrix(currTranslation[i]) *
                                         rotationMatrix(currRotation[i])       *
                                         shearMatrix(currShear[i])             *
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