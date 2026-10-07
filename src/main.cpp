#include "math_lib.h"

#include "input.h"

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
///                                     Crossplatform Functions
/// ##############################################################################################
#include <chrono>
double getDeltaTime();

int main()
{
        rng_seed(time(NULL), 1);
        getDeltaTime();

        BumpAllocator transientStorage = makeBumpAllocator(MB(2000));
        BumpAllocator persistentStorage = makeBumpAllocator(MB(50));

        input = (Input*)bumpAlloc(&persistentStorage, sizeof(Input));
        renderData = (RenderData*)bumpAlloc(&persistentStorage, sizeof(RenderData));
        *renderData = {};
        renderData->pointCount = MAX_POINTS;
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
                        static float displayTimer = 0.0f;
                        displayTimer += deltaTime;
                        if(displayTimer > 1.5f)
                        {
                                displayTimer = 0.0f;
                                
                                SM_TRACE(
                                                "\nms:        %f \n" 
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

                        static float resetTimer = 0.0f;
                        resetTimer += deltaTime;
                        if(resetTimer > 10.0f)
                        {
                                resetTimer = 0.0f;
                                minFps = 0.0f;
                                maxFps = 0.0f;
                        }
                }

                platformUpdateWindow();

                static float timer = 0.0f;
                timer += deltaTime;

                if(timer >= 3.0f)
                {
                        timer = 0.0f;
                        ifs.generateNewParameters();
                }

                glRender(&transientStorage, ifs, deltaTime, timeSeconds);

                platformSwapBuffers();

                if(key_pressed_this_frame(KEY_G)) ifs.generateNewParameters();

                transientStorage.used = 0;
        }

        return 0;
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