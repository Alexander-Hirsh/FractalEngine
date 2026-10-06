#include "sim.h"

#include "assets.h"
#include "math_lib.h"


/// ##############################################################################################
///                                     Game Constants
/// ##############################################################################################
const int WORLD_WIDTH = RENDER_WIDTH;
const int WORLD_HEIGHT = RENDER_HEIGHT;


/// ##############################################################################################
///                                     Game Functions
/// ##############################################################################################
EXPORT_FN void updateSim(RenderData* renderDataIn, Input* inputIn, IFS* ifs, float deltaTimeIn)
{
        renderData = renderDataIn;
        input = inputIn;

        renderData->pointCount = MAX_POINTS;

        static float timer = 0.0f;
        timer += deltaTimeIn;

        if(timer >= 2.0f)
        {
                timer = 0.0f;
                ifs->generateNewParameters();
        }
}