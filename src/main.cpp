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
///                                     Global Variables
/// ##############################################################################################
float deltaTime = 0.0f;
static double minFps = 0.0;
static double maxFps = 0.0;
static double fpsCapped = 0.0;
bool screenshotFrame = false;

/// ##############################################################################################
///                                     Crossplatform Functions
/// ##############################################################################################
#include <chrono>

double getDeltaTime();
void drawParameterGUI(PARAMETERS& params, IFS& ifs);
void drawIFSTransformsGUI(IFS& ifs);

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
        PARAMETERS params;

        ifs.generateNewParameters(params);
        
        platformFillKeycodeLookup();
        if(!platformCreateWindow(RENDER_WIDTH, RENDER_HEIGHT, L"FractalEngine"))
        {
                SM_ERROR("Failed to create platform window");
                return 1;
        }
        input->screenSize.x = RENDER_WIDTH;
        input->screenSize.y = RENDER_HEIGHT;
        
        if(!glInit(&transientStorage)) return 1;

        static float timeSeconds = 0.0f;
        while(running)
        {
                deltaTime = (float)getDeltaTime();
                timeSeconds += deltaTime;

                double currentFps = deltaTime > 0.0f ? 1.0 / deltaTime : 0.0;

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

                                fpsCapped = currentFps;
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

                if(timer >= params.timeToChange && !params.pause)
                {
                        timer = 0.0f;
                        ifs.generateNewParameters(params);
                }

                renderData->gameCamera.updateCamera(*input);

                if(key_pressed_this_frame(KEY_G)) ifs.generateNewParameters(params);
                if(key_pressed_this_frame(KEY_SPACE)) params.pause = !params.pause;
                if(key_pressed_this_frame(KEY_H)) params.hideGUI = !params.hideGUI;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplWin32_NewFrame();

                // Draw GUI
                ImGui::NewFrame();
                if(!params.hideGUI)
                {
                        drawParameterGUI(params, ifs);
                        drawIFSTransformsGUI(ifs);
                }

                int dispatchSizeSave = params.dispatches;
                if(screenshotFrame) params.dispatches = params.screenshotDispatches;

                // Update matricies
                ifs.generateTransformMatrix(params.pause, deltaTime);
                glRender(&transientStorage, ifs, params, deltaTime, timeSeconds);

                if(screenshotFrame)
                {
                        takeScreenshot(glContext.FBOID, std::to_string(std::time(0)));
                        params.dispatches = dispatchSizeSave;
                        screenshotFrame = false;
                }

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                platformSwapBuffers();

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

void drawParameterGUI(PARAMETERS& params, IFS& ifs)
{
        ImGui::Begin("Parameters");

        ImGui::SeparatorText("Info");

        ImGui::Text("Open source Iterated function system simulation.\nSimply a procedural Fractal generator");
        ImGui::Text("\nRMB: Rotate\nScroll: Zoom in/out\nSpace: Pause\nH: Hide interface\nG: Generate new");
        ImGui::Text("\nFPS | C:%f | N:%f | X:%f", fpsCapped, minFps, maxFps);
        ImGui::Text("Particles: %iM over %i dispatches", int(MAX_POINTS/1000000), int(params.dispatches));
        ImGui::Text("MS | %f", 1.0 / deltaTime);

        ImGui::SeparatorText("Particles");
        ImGui::Text("Number of times %iM particle buffer gets drawn\nMore dispatches = Less noise = More GPU work = Less FPS", int(MAX_POINTS/1000000));
        ImGui::SliderInt("Particle Dispatches", &params.dispatches, 1, 100);
        ImGui::SetItemTooltip("Affects real time simulation");
        ImGui::SliderInt("Screenshot Dispatches", &params.screenshotDispatches, 1, 100000);
        ImGui::SetItemTooltip("Affects screenshot draw.\nTook about 2 minutes with Laptop 3070 on 10k dispatches for one screenshot");

        ImGui::SliderFloat("Time to new fractal", &params.timeToChange, 0.1, 20.0);
        ImGui::SetItemTooltip("How often new fractal is generated");
        ImGui::SliderFloat("Lerp curve", &params.iCurve, 0.01, 0.99);
        ImGui::SetItemTooltip("How fast it transitions from old one");

        ImGui::SeparatorText("Buttons");

        if(ImGui::Button("Generate")) ifs.generateNewParameters(params);
        ImGui::SameLine();
        if(ImGui::Button("Pause")) params.pause = !params.pause;
        ImGui::SameLine();
        if(ImGui::Button("Screenshot")) screenshotFrame = true;
        ImGui::SameLine();
        if(ImGui::Button("Exit")) running = false;

        ImGui::SeparatorText("Colors");

        ImGui::ColorEdit4("Light color", (float*)&params.lightColor, ImGuiColorEditFlags_PickerHueWheel);
        ImGui::ColorEdit4("Shadow color", (float*)&params.shadowColor, ImGuiColorEditFlags_PickerHueWheel);
        ImGui::SliderFloat("Shadow threshold", &params.shadowThreshold, 0.0f, 3.0f);

        ImGui::SeparatorText("Bounds");
        ImGui::Text("Min and Max values that can be generated for each transformation");
        ImGui::SliderFloat2("Scale Bounds", (float*)&params.scaleBounds, 0.1f, 0.9f);
        ImGui::SliderFloat2("Shear Bounds", (float*)&params.shearBounds, 0.0f, 0.5f);
        ImGui::SliderFloat2("Rotation Bounds", (float*)&params.rotationBounds, -360.0f, 360.0f);
        ImGui::SliderFloat2("Translation Bounds", (float*)&params.translationBounds, 0.0f, 20.0f);

        ImGui::SeparatorText("Post Processing");

        ImGui::Checkbox("Use Kuwahara Filter", &params.useKuwahara);
        ImGui::SetItemTooltip("Denoising filter");
        ImGui::Checkbox("Use Bloom", &params.useBloom);
        ImGui::SetItemTooltip("You want some bloom? This thing does some bloom");
        ImGui::SliderFloat("Bloom Strength", &params.bloomStrength, 0.01f, 1.0f);
        ImGui::SliderFloat("Gamma", &params.gamma, 0.0f, 3.0f);

        ImGui::End();
}

void drawIFSTransformsGUI(IFS& ifs)
{
        ImGui::Begin("Transforms");

        ImGui::SeparatorText("Scale");
        for(int i = 0; i < ifs.currNumOfMatrix; i++)
        {
                std::string sliderName = "Scale" + std::to_string(i); 
                ImGui::SliderFloat3(sliderName.c_str(), (float*)&ifs.currScale[i], -0.0, 0.99);
        }

        ImGui::SeparatorText("Shear");
        for(int i = 0; i < ifs.currNumOfMatrix; i++)
        {
        std::string sliderName = "Shear" + std::to_string(i); 
                ImGui::SliderFloat3(sliderName.c_str(), (float*)&ifs.currShear[i], -0.2, 0.2);
        }

        ImGui::SeparatorText("Rotation");
        ImGui::Text("Note that rotations are in radians and not degrees");
        for(int i = 0; i < ifs.currNumOfMatrix; i++)
        {
                std::string sliderName = "Rotation" + std::to_string(i); 
                ImGui::SliderFloat3(sliderName.c_str(), (float*)&ifs.currRotation[i], degToRad(-360.0), degToRad(360.0));
        }

        ImGui::SeparatorText("Translation");
        for(int i = 0; i < ifs.currNumOfMatrix; i++)
        {
                std::string sliderName = "Translation" + std::to_string(i); 
                ImGui::SliderFloat3(sliderName.c_str(), (float*)&ifs.currTranslation[i], -25.0, 25.0);
        }

        ImGui::End();
}