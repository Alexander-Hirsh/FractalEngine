#include "ogl_renderer.h"

#include "render_interface.h"

/// ##############################################################################################
///                                     OpenGL Structs
/// ##############################################################################################
struct GLContext
{
        // Programs
        GLuint pointProgramID;          // Draws point cloud approximation
        GLuint computeProgramID;        // Computes IFS points positions
        GLuint voxelProgramID;          // Displays IFS AO grid occupation
        GLuint boundsProgramID;         // Computes IFS bounds
        GLuint fullscreenProgramID;     // Post process program
        GLuint downscaleProgramID;      // Downscaler for blur textures
        GLuint upscaleProgramID;        // Upscaler for blur textures
        GLuint reductionProgramID;      // Precomputes AO grid for sampling

        // SSBO Buffers
        GLuint pointVertexArrayID;
        GLuint pointBufferID;
        GLuint matrixBufferID;
        GLuint boundsBufferID;
        GLuint AOgridBufferID;
        GLuint sortedAOGridBufferID;
        
        GLuint FBOID;
        GLuint RBOID;
        GLuint bloomFBOID;

        // Point cloud uniforms
        GLuint projectionID;
        GLuint vertexAOgridSizeID;

        // Compute uniforms
        GLuint computeTimeID;
        GLuint computePointCountID;
        GLuint computeMatrixCountID;
        GLuint computeAOgridSizeID;
        GLuint dispatchID;

        // Reduction uniforms
        GLuint reductionAOgridSizeID;

        //Bounds uniforms
        GLuint boundsMatrixCountID;

        // Voxel Debug uniforms
        GLuint vd_projectMatID;

        // Post processing uniforms
        GLuint pp_framebufferTextureID;
        GLuint pp_bloomTextureID;
        GLuint downscaleSrcTextureID;
        GLuint downscaleSrcResolutionID;
        GLuint upscaleSrcTextureID;
        GLuint upscaleFilterRadiusID;

        // Texture IDs
        GLuint framebufferTextureID;
        std::vector<bloomMip> bloomMipChain;

        // Parameter uniforms
        GLuint lightColor;
        GLuint shadowColor;
        GLuint shadowThreshold;
        GLint useKuwahara;
        GLint useBloom;
        GLuint bloomStrength;

};


/// ##############################################################################################
///                                     OpenGl Globals
/// ##############################################################################################
static GLContext glContext;
static const uint32_t emptyAOGrid[AO_GRID_32BIT_GRID_SIZE] = {};

/// ##############################################################################################
///                                     OpenGL Functions
/// ##############################################################################################
static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                       GLsizei length, const GLchar* message, const void* user)
{
        if(
        //severity == GL_DEBUG_SEVERITY_LOW || 
        // severity == GL_DEBUG_SEVERITY_MEDIUM ||
        severity == GL_DEBUG_SEVERITY_HIGH)
        {
                SM_ASSERT(false, "OpenGL Error: %s", message);
        }
        else
        {
                SM_TRACE((char*)message);
        }
}



GLuint glShaderInit(int shaderType, const char* shaderPath, BumpAllocator* transientStorage)
{
        int fileSize = 0;
        char* shaderData = readFile(shaderPath, &fileSize, transientStorage);
        if(!shaderData)
        {
                SM_ASSERT(false, "Failed to load %s shader", shaderPath);
                return 0;
        }

        GLuint shaderID = glCreateShader(shaderType);
        glShaderSource(shaderID, 1, &shaderData, 0);
        glCompileShader(shaderID);

        // Check if shader actually compiled
        {
                int success;
                char shaderLog[2048] = {};

                glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
                if(!success)
                {
                        glGetShaderInfoLog(shaderID, 2048, 0, shaderLog);
                        SM_ASSERT(false, "Failed to compile %s Shader, Error: %s", shaderPath, shaderLog);
                        glDeleteShader(shaderID);
                        return 0;
                }
        }

        return shaderID;
}

GLuint glCreateShaderProgram(BumpAllocator* transientStorage, const char* vertexShaderName,
                             const char* fragmentShaderName = nullptr)
{
        if(!fragmentShaderName) fragmentShaderName = vertexShaderName;

        GLuint vertShaderID = glShaderInit(GL_VERTEX_SHADER,   
                (std::string("assets/shaders/") + vertexShaderName + std::string(".vert")).c_str(),
                transientStorage);
        GLuint fragShaderID = glShaderInit(GL_FRAGMENT_SHADER, 
                (std::string("assets/shaders/") + fragmentShaderName + std::string(".frag")).c_str(),
                transientStorage);

        if(!vertShaderID || !fragShaderID)
        {
                if(vertShaderID) glDeleteShader(vertShaderID);
                if(fragShaderID) glDeleteShader(fragShaderID);
                return 0;
        }

        GLuint programID = glCreateProgram();
        glAttachShader(programID, vertShaderID);
        glAttachShader(programID, fragShaderID);
        glLinkProgram(programID);
        glDetachShader(programID, vertShaderID);
        glDetachShader(programID, fragShaderID);
        glDeleteShader(vertShaderID);
        glDeleteShader(fragShaderID);

        GLint success = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if(!success)
        {
                char programInfoLog[512] = {};
                glGetProgramInfoLog(programID, sizeof(programInfoLog), 0, programInfoLog);
                SM_ASSERT(false, "Failed to link shader program: %s", programInfoLog);
                glDeleteProgram(programID);
                return 0;
        }

        return programID;
}

GLuint glCreateComputeProgram(BumpAllocator* transientStorage, const char* shaderName)
{
        GLuint compShaderID = glShaderInit(GL_COMPUTE_SHADER,
                (std::string("assets/shaders/") + shaderName + std::string(".comp")).c_str(),
                transientStorage);

        if(!compShaderID) return 0;

        GLuint programID = glCreateProgram();
        glAttachShader(programID, compShaderID);
        glLinkProgram(programID);
        glDetachShader(programID, compShaderID);
        glDeleteShader(compShaderID);

        GLint success = GL_FALSE;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if(!success)
        {
                char programInfoLog[512] = {};
                glGetProgramInfoLog(programID, sizeof(programInfoLog), 0, programInfoLog);
                SM_ASSERT(false, "Failed to link compute shader: %s", programInfoLog);
                glDeleteProgram(programID);
                return 0;
        }

        return programID;
}

bool glInit(BumpAllocator* transientStorage)
{
        loadGlFunctions();

        glDebugMessageCallback(&glDebugCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glEnable(GL_DEBUG_OUTPUT);

        glContext.pointProgramID      = glCreateShaderProgram (transientStorage, POINT_PROGRAM_NAME);
        glContext.computeProgramID    = glCreateComputeProgram(transientStorage, IFS_PROGRAM_NAME);
        glContext.boundsProgramID     = glCreateComputeProgram(transientStorage, BOUND_PROGRAM_NAME);
        glContext.fullscreenProgramID = glCreateShaderProgram (transientStorage, FULLSCREEN_PROGRAM_NAME);
        glContext.downscaleProgramID  = glCreateShaderProgram (transientStorage, FULLSCREEN_PROGRAM_NAME, DOWNSCALE_PROGRAM_NAME);
        glContext.upscaleProgramID    = glCreateShaderProgram (transientStorage, FULLSCREEN_PROGRAM_NAME, UPSCALE_PROGRAM_NAME);
        glContext.reductionProgramID  = glCreateComputeProgram(transientStorage, REDUCTION_PROGRAM_NAME);

        if(!glContext.pointProgramID       || 
           !glContext.computeProgramID     ||
           !glContext.boundsProgramID      ||
           !glContext.fullscreenProgramID  ||
           !glContext.downscaleProgramID   ||
           !glContext.upscaleProgramID     ||
           !glContext.reductionProgramID)  return false;

        glGenVertexArrays(1, &glContext.pointVertexArrayID);
        glBindVertexArray(glContext.pointVertexArrayID);


        
        // CONFIGURE POINT BUFFER TO COMPUTE SHADER TO WORK ON
        {
                glGenBuffers(1, &glContext.pointBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.pointBufferID);

                uint64_t pointBufferCapacity = MAX_POINTS + BOUNDS_DEBUG_VERTEX_COUNT;
                vec4* initialPoints = (vec4*)bumpAlloc(transientStorage, sizeof(vec4) * pointBufferCapacity);
                for(uint64_t i = 0; i < pointBufferCapacity; i++)
                {
                        initialPoints[i] = {0.0, 0.0, 0.0, 1.0f};
                }

                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * pointBufferCapacity,
                        initialPoints, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.pointBufferID);
        }

        // CONFIGURE TRANSFORM MATRICES TO COMPUTE SHADER
        {
                glGenBuffers(1, &glContext.matrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.matrixBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat4) * IFS::maxNumOfMatrix,
                             nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glContext.matrixBufferID);
        }

        // CONFIGURE AMBIENT OCCLUSION GRIDS FOR LIGHTING
        {
                // Regular grid
                glGenBuffers(1, &glContext.AOgridBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.AOgridBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER,
                             sizeof(uint32_t) * AO_GRID_32BIT_GRID_SIZE, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glContext.AOgridBufferID);

                // Sorted grid
                glGenBuffers(1, &glContext.sortedAOGridBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.sortedAOGridBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER,
                             sizeof(uint32_t) * AO_GRID_CELL_COUNT, nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, glContext.sortedAOGridBufferID);
        }

        // CONFIGURE AABB BOUNDS BUFFER
        {
                struct BoundsGPU
                {
                        vec4 minPoint;
                        vec4 maxPoint;
                        uint32_t valid;
                };

                glGenBuffers(1, &glContext.boundsBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.boundsBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BoundsGPU),
                             nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, glContext.boundsBufferID);
        }

        // CONFIGURE FRAMEBUFFER OBJECT
        {
                glGenFramebuffers(1, &glContext.FBOID);
                glBindFramebuffer(GL_FRAMEBUFFER, glContext.FBOID);

                glGenTextures(1, &glContext.framebufferTextureID);
                glBindTexture(GL_TEXTURE_2D, glContext.framebufferTextureID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT, 0,
                             GL_RGBA, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       glContext.framebufferTextureID, 0);

                glGenRenderbuffers(1, &glContext.RBOID);
                glBindRenderbuffer(GL_RENDERBUFFER, glContext.RBOID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, RENDER_WIDTH, RENDER_HEIGHT);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glContext.RBOID);

                GLenum attachment = GL_COLOR_ATTACHMENT0;
                glDrawBuffers(1, &attachment);

                GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                SM_ASSERT(framebufferStatus == GL_FRAMEBUFFER_COMPLETE,
                          "Framebuffer ERROR: status 0x%X", framebufferStatus);
        }

        // CONFIGURE BLOOM FRAMEBUFFER OBJECT
        {
                glGenFramebuffers(1, &glContext.bloomFBOID);
                glBindFramebuffer(GL_FRAMEBUFFER, glContext.bloomFBOID);

                ivec2 mipIntSize = {(int)RENDER_WIDTH, (int)RENDER_HEIGHT};

                for(int i = 0; i < MIP_CHAIN_LENGTH; i++)
                {
                        bloomMip mip;

                        mipIntSize.x = max(1, mipIntSize.x / MIP_RATIO);
                        mipIntSize.y = max(1, mipIntSize.y / MIP_RATIO);
                        mip.iSize = mipIntSize;

                        glGenTextures(1, &mip.texture);
                        glBindTexture(GL_TEXTURE_2D, mip.texture);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
                                     mipIntSize.x, mipIntSize.y,
                                     0, GL_RGB, GL_FLOAT, nullptr);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                        glContext.bloomMipChain.emplace_back(mip);
                }

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       glContext.bloomMipChain[0].texture, 0);

                GLenum attachment = GL_COLOR_ATTACHMENT0;
                glDrawBuffers(1, &attachment);

                GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                SM_ASSERT(framebufferStatus == GL_FRAMEBUFFER_COMPLETE,
                          "Bloom framebuffer ERROR: status 0x%X", framebufferStatus);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Compute uniforms
        glContext.computeTimeID = glGetUniformLocation(glContext.computeProgramID, "TIME");
        glContext.computePointCountID = glGetUniformLocation(glContext.computeProgramID, "pointCount");
        glContext.computeMatrixCountID = glGetUniformLocation(glContext.computeProgramID, "matrixCount");
        glContext.computeAOgridSizeID = glGetUniformLocation(glContext.computeProgramID, "AOgridSize");
        glContext.dispatchID = glGetUniformLocation(glContext.computeProgramID, "dispatchID");

        // Point cloud uniforms
        glContext.projectionID = glGetUniformLocation(glContext.pointProgramID, "cameraSpaceTransform");
        glContext.vertexAOgridSizeID = glGetUniformLocation(glContext.pointProgramID, "AOgridSize");

        // Bounds compute uniforms
        glContext.boundsMatrixCountID = glGetUniformLocation(glContext.boundsProgramID, "matrixCount");

        // Post processing uniforms
        glContext.pp_framebufferTextureID = glGetUniformLocation(glContext.fullscreenProgramID, "screenTex");
        glContext.pp_bloomTextureID = glGetUniformLocation(glContext.fullscreenProgramID, "bloomTex");
        glContext.downscaleSrcTextureID = glGetUniformLocation(glContext.downscaleProgramID, "srcTexture");
        glContext.downscaleSrcResolutionID = glGetUniformLocation(glContext.downscaleProgramID, "srcResolution");
        glContext.upscaleSrcTextureID = glGetUniformLocation(glContext.upscaleProgramID, "srcTexture");
        glContext.upscaleFilterRadiusID = glGetUniformLocation(glContext.upscaleProgramID, "filterRadius");

        glContext.reductionAOgridSizeID = glGetUniformLocation(glContext.reductionProgramID, "gridSize");

        // Parameter uniforms
        glContext.lightColor = glGetUniformLocation(glContext.pointProgramID, "lightColor");
        glContext.shadowColor = glGetUniformLocation(glContext.pointProgramID, "shadowColor");
        glContext.shadowThreshold = glGetUniformLocation(glContext.pointProgramID, "shadowThreshold");
        glContext.useKuwahara = glGetUniformLocation(glContext.fullscreenProgramID, "useKuwahara");
        glContext.useBloom = glGetUniformLocation(glContext.fullscreenProgramID, "useBloom");
        glContext.bloomStrength = glGetUniformLocation(glContext.fullscreenProgramID, "bloomStrength");

        glEnable(GL_FRAMEBUFFER_SRGB);
        // glEnable(GL_PROGRAM_POINT_SIZE);

        return true;
}

void glRender(BumpAllocator* transientStorage, IFS& ifs, PARAMETERS params, float deltaTime, float timeSeconds)
{
        
        if(renderData->pointCount < 0 || renderData->pointCount > MAX_POINTS)
        {
                SM_ASSERT(false, "Point count %d exceeds the render capacity of %d",
                        renderData->pointCount, MAX_POINTS);
                return;
        }

        Camera3D& camera = renderData->gameCamera;

        float top = camera.nearPlane * tanf(degToRad(camera.fov / 2.0f));
        float halfWidth = top * ((float)input->screenSize.x / input->screenSize.y);

        mat4 projection = perspectiveProjection(-halfWidth, halfWidth, top, -top, camera.nearPlane, camera.farPlane);
        mat4 view = lookAt(camera.forward, camera.right * -1.0f, camera.up * -1.0f) *
                    translationMatrix(camera.position * -1.0f);

        mat4 cameraSpaceTransform = projection * view;

        glViewport(0, 0, input->screenSize.x, input->screenSize.y);
        glBindFramebuffer(GL_FRAMEBUFFER, glContext.FBOID);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.pointBufferID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glContext.matrixBufferID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glContext.AOgridBufferID);

        glEnable(GL_DEPTH_TEST);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.matrixBufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                        (GLsizeiptr)(sizeof(mat4) * ifs.currNumOfMatrix),
                        ifs.IFSMatrices);

        // Clear the previous frame's cell occupancy before computing this frame.
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.AOgridBufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                        sizeof(emptyAOGrid), emptyAOGrid);

        // Compute IFS AA bounding box
        {
                glUseProgram(glContext.boundsProgramID);
                glUniform1i(glContext.boundsMatrixCountID, ifs.currNumOfMatrix);

                glDispatchCompute(1, 1, 1);

                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        for(int d = 0; d < params.dispatches; d++)
        {
                // Compute point cloud positions
                {        
                        glUseProgram(glContext.computeProgramID);
                        glUniform1i(glContext.computePointCountID, renderData->pointCount);
                        glUniform1f(glContext.computeTimeID, timeSeconds);
                        glUniform1i(glContext.computeMatrixCountID, ifs.currNumOfMatrix);
                        glUniform1i(glContext.computeAOgridSizeID, AO_GRID_SIDE_SIZE);
                        glUniform1i(glContext.dispatchID, d);

                        GLuint workGroupCount = (renderData->pointCount + POINT_COMPUTE_LOCAL_SIZE - 1) /
                                                                POINT_COMPUTE_LOCAL_SIZE;

                        glDispatchCompute(workGroupCount, 1, 1);

                        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                // Use reduction program to precompute grid neighbors
                if(d == 0) // Compute bounds only on first dispatch
                {
                        glUseProgram(glContext.reductionProgramID);
                        glUniform1i(glContext.reductionAOgridSizeID, AO_GRID_SIDE_SIZE);

                        glDispatchCompute(AO_GRID_SIDE_SIZE / 32 / 4, AO_GRID_SIDE_SIZE / 8, AO_GRID_SIDE_SIZE / 8);   // = (1, 16, 16)
                        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
                }

                // Use point cloud program to draw computed points
                {        
                        glUseProgram(glContext.pointProgramID);
                        
                        glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &cameraSpaceTransform.ax);
                        glUniform1i(glContext.vertexAOgridSizeID, AO_GRID_SIDE_SIZE);
                        glUniform4fv(glContext.lightColor, 1, params.lightColor.values);
                        glUniform4fv(glContext.shadowColor, 1, params.shadowColor.values);
                        glUniform1f(glContext.shadowThreshold, params.shadowThreshold);

                        glBindVertexArray(glContext.pointVertexArrayID);
                        glDrawArrays(GL_POINTS, 0, renderData->pointCount);
                        glDrawArrays(GL_LINES, renderData->pointCount, BOUNDS_DEBUG_VERTEX_COUNT);
                }
        }

        // Downsample the complete HDR scene into the bloom mip chain.
        {
                glDisable(GL_DEPTH_TEST);
                glBindFramebuffer(GL_FRAMEBUFFER, glContext.bloomFBOID);
                glUseProgram(glContext.downscaleProgramID);
                glUniform1i(glContext.downscaleSrcTextureID, 0);
                glActiveTexture(GL_TEXTURE0);

                GLuint sourceTexture = glContext.framebufferTextureID;
                int sourceWidth = RENDER_WIDTH;
                int sourceHeight = RENDER_HEIGHT;

                for(size_t i = 0; i < glContext.bloomMipChain.size(); i++)
                {
                        bloomMip& mip = glContext.bloomMipChain[i];
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                               mip.texture, 0);
                        glViewport(0, 0, mip.iSize.x, mip.iSize.y);
                        glBindTexture(GL_TEXTURE_2D, sourceTexture);
                        vec2 sourceResolution = {(float)sourceWidth, (float)sourceHeight};
                        glUniform2fv(glContext.downscaleSrcResolutionID, 1, &sourceResolution.x);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                        sourceTexture = mip.texture;
                        sourceWidth = mip.iSize.x;
                        sourceHeight = mip.iSize.y;
                }
        }

        // Progressively blur and add each smaller mip into the next larger mip.
        {
                glUseProgram(glContext.upscaleProgramID);
                glUniform1i(glContext.upscaleSrcTextureID, 0);
                glActiveTexture(GL_TEXTURE0);
                glEnable(GL_BLEND);
                glBlendEquation(GL_FUNC_ADD);
                glBlendFunci(0, GL_ONE, GL_ONE);

                for(int i = (int)glContext.bloomMipChain.size() - 1; i > 0; i--)
                {
                        bloomMip& sourceMip = glContext.bloomMipChain[i];
                        bloomMip& targetMip = glContext.bloomMipChain[i - 1];
                        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                               targetMip.texture, 0);
                        glViewport(0, 0, targetMip.iSize.x, targetMip.iSize.y);
                        glBindTexture(GL_TEXTURE_2D, sourceMip.texture);
                        vec2 filterRadius = {1.0f / sourceMip.iSize.x,
                                             1.0f / sourceMip.iSize.y};
                        glUniform2fv(glContext.upscaleFilterRadiusID, 1, &filterRadius.x);
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                glDisable(GL_BLEND);
        }

        // Mix the HDR scene with the completed bloom texture for display.
        {
                glUseProgram(glContext.fullscreenProgramID);
                glUniform1i(glContext.pp_framebufferTextureID, 0);
                glUniform1i(glContext.pp_bloomTextureID, 1);

                glUniform1i(glContext.useKuwahara, params.useKuwahara);
                glUniform1i(glContext.useBloom, params.useBloom);
                glUniform1f(glContext.bloomStrength, params.bloomStrength);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, input->screenSize.x, input->screenSize.y);
                glDisable(GL_DEPTH_TEST);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glContext.framebufferTextureID);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, glContext.bloomMipChain[0].texture);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
}

void takeScreenshot(GLuint FBOID, std::string timestamp)
{
        std::vector<unsigned char> pixels(RENDER_WIDTH * RENDER_HEIGHT * 4);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);   // the RGBA8 one
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, RENDER_WIDTH, RENDER_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // OpenGL's origin is bottom-left, PNG's is top-left
        stbi_flip_vertically_on_write(1);
        std::string fileName = "screenshots/" + timestamp + "capture.png";
        stbi_write_png(fileName.c_str(), RENDER_WIDTH, RENDER_HEIGHT, 4, pixels.data(), RENDER_WIDTH * 4);
}