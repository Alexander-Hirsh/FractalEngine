#pragma once

#include "input.h"
#include "math_lib.h"
#include "render_interface.h"


/// ##############################################################################################
///                                     Type Enums
/// ##############################################################################################


/// ##############################################################################################
///                                     Sim Structs
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

        float iCurve = 0.99f;

        void generateNewParameters()
        {
                currNumOfMatrix = (int)(rng_f() * (maxNumOfMatrix - 1)) + 2;

                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        newScale[i] = randVec3(0.65f, 0.75f);
                        newRotation[i] = randVec3(-degToRad(80), degToRad(80));
                        newShear[i] = randVec3(-0.15f, 0.15f);
                        newTranslation[i] = randVec3(-0.5f, 0.5f);
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
///                                     Sim Globals
/// ##############################################################################################
/// ##############################################################################################
///                                     Sim Functions (exposed)
/// ##############################################################################################
extern "C"
{
        EXPORT_FN void updateSim(RenderData* renderDataIn, Input* inputIn, IFS* ifs, float deltaTimeIn);
}