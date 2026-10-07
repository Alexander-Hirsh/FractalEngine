#include "math_lib.h"

#include "input.h"

#include "sim.h"

#include "platform.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

#include <time.h>

#ifdef _WIN32
#include "win32_platform.cpp"
#endif

#include "ogl_renderer.cpp"

/// ##############################################################################################
///                                     Game DLL shit
/// ##############################################################################################
typedef decltype(updateSim) updateSimType;
static updateSimType* updateSimPtr;


/// ##############################################################################################
///                                     Crossplatform Functions
/// ##############################################################################################
#include <chrono>
double getDeltaTime();
void reloadSimDll(BumpAllocator* transientStorage);

int main()
{
        rng_seed(time(NULL), 1);
        getDeltaTime();

        BumpAllocator transientStorage = makeBumpAllocator(MB(2000));
        BumpAllocator persistentStorage = makeBumpAllocator(MB(50));

        input = (Input*)bumpAlloc(&persistentStorage, sizeof(Input));
        renderData = (RenderData*)bumpAlloc(&persistentStorage, sizeof(RenderData));
        *renderData = {};
        IFS ifs = {};
        ifs.generateNewParameters();
        
        platformFillKeycodeLookup();
        platformCreateWindow(RENDER_WIDTH, RENDER_HEIGHT, L"FractalEngine");
        input->screenSize.x = RENDER_WIDTH;
        input->screenSize.y = RENDER_HEIGHT;
        
        glInit(&transientStorage);

        static float timeSeconds = 0.0f;
        while(running)
        {
                float deltaTime = (float)getDeltaTime();
                timeSeconds += deltaTime;

                double currentFps = deltaTime > 0.0f ? 1.0 / deltaTime : 0.0;
                static double minFps = 0.0;
                static double maxFps = 0.0;
                if(currentFps > 0.0)
                {
                        if(minFps == 0.0 || currentFps < minFps) minFps = currentFps;
                        if(currentFps > maxFps) maxFps = currentFps;
                }


                // Performance stats
                {
                        static float timer = 0.0f;
                        timer += deltaTime;
                        if(timer > 1.5f)
                        {
                                timer = 0.0f;
                                
                                SM_TRACE(
                                                "\nms:         %f \n" 
                                                "current FPS: %f \n"
                                                "min FPS:     %f \n"
                                                "max FPS:     %f \n", 
                                                
                                                deltaTime * 1000.0f, 
                                                currentFps,
                                                minFps,
                                                maxFps
                                        );

                                if(deltaTime > 0.1f) deltaTime = 0.1f;
                        }
                }

                reloadSimDll(&transientStorage);

                platformUpdateWindow();
                updateSim(renderData, input, &ifs, deltaTime);

                glRender(&transientStorage, ifs, deltaTime, timeSeconds);

                platformSwapBuffers();

                if(key_pressed_this_frame(KEY_G)) ifs.generateNewParameters();

                transientStorage.used = 0;
        }

        return 0;
}

void updateSim(RenderData* renderDataIn, Input* inputIn, IFS* ifs, float deltaTimeIn)
{
        updateSimPtr(renderDataIn, inputIn, ifs, deltaTimeIn);
}

double getDeltaTime()
{
        // Only executed once when entering the function (static)
        static auto lastTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();

        // seconds
        double delta = std::chrono::duration<double>(currentTime - lastTime).count(); 
        lastTime = currentTime; 

        return delta;
}

void reloadSimDll(BumpAllocator* transientStorage)
{
        static void* simDLL;
        static long long lastEditTimestampSimDLL;

        long long currentTimestampSimDLL = getTimestamp("sim.dll");

        if(updateSimPtr == nullptr || currentTimestampSimDLL > lastEditTimestampSimDLL)
        {
                if(simDLL)
                {
                        bool freeResult = platformFreeDynamicLibrary(simDLL);
                        SM_ASSERT(freeResult, "Failed to free sim.dll");

                        simDLL = nullptr;
                        updateSimPtr = nullptr;
                        SM_TRACE("Freed sim.dll");
                }

                while(!copyFile("sim.dll", "sim_load.dll", transientStorage))
                {
                        SM_TRACE("Failed to copy sim.dll into sim_load.dll, retrying in 10ms");
                        Sleep(10);
                }
                SM_TRACE("Copied sim.dll into sim_load.dll");

                simDLL = platformLoadDynamicLibrary("sim_load.dll");
                SM_ASSERT(simDLL, "Failed to load sim.dll")

                updateSimPtr = (updateSimType*)platformLoadDynamicFunction(simDLL, "updateSim");
                SM_ASSERT(updateSimPtr, "Failed to load updateSim function");
                lastEditTimestampSimDLL = currentTimestampSimDLL;

        }
}
