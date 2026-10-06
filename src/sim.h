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
        static const int maxNumOfMatrix = 7;

        vec3 currScale[maxNumOfMatrix] = {};
        vec3 currRotation[maxNumOfMatrix] = {};
        vec3 currShear[maxNumOfMatrix] = {};
        vec3 currTranslation[maxNumOfMatrix] = {};

        vec3 newScale[maxNumOfMatrix] = {};
        vec3 newRotation[maxNumOfMatrix] = {};
        vec3 newShear[maxNumOfMatrix] = {};
        vec3 newTranslation[maxNumOfMatrix] = {};

        mat4 IFSMatrices[maxNumOfMatrix] = {};

        void generateNewParameters()
        {
                currNumOfMatrix = (int)(rng_f() * (maxNumOfMatrix - 1)) + 2;

                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        newScale[i] = randVec3(0.35f, 0.5f);
                        newRotation[i] = randVec3(-degToRad(360), degToRad(360));
                        newShear[i] = randVec3(-0.1f, 0.1f);
                        newTranslation[i] = randVec3(-0.5f, 0.5f);
                        newTranslation[i].z = -1.0f - rng_f();

                        currScale[i] = newScale[i];
                        currRotation[i] = newRotation[i];
                        currShear[i] = newShear[i];
                        currTranslation[i] = newTranslation[i];
                }

                generateTransformMatrix();
        }

        void generateTransformMatrix()
        {
                for(int i = 0; i < currNumOfMatrix; i++)
                {
                        IFSMatrices[i] = constrTranslationMatrix(currTranslation[i]) *
                                          constrRotationMatrix(currRotation[i])    *
                                          constrShearMatrix(currShear[i])          *
                                          constrScaleMatrix(currScale[i]);
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