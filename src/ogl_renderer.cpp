#include "ogl_renderer.h"

#include "render_interface.h"


/// ##############################################################################################
///                                     OpenGL Structs
/// ##############################################################################################
struct GLContext
{
        // Programs
        GLuint pointProgramID;
        GLuint computeProgramID;
        GLuint voxelProgramID;
        GLuint boundsProgramID;
        GLuint fullscreenProgramID;

        // SSBO Buffers
        GLuint pointVertexArrayID;
        GLuint pointBufferID;
        GLuint matrixBufferID;
        GLuint boundsBufferID;
        GLuint AOgridBufferID;
        
        GLuint FBOID;
        GLuint RBOID;

        // Point cloud uniforms
        GLuint projectionID;
        GLuint vertexAOgridSizeID;

        // Compute uniforms
        GLuint computeTimeID;
        GLuint computePointCountID;
        GLuint computeMatrixCountID;
        GLuint computeAOgridSizeID;

        //Bounds uniforms
        GLuint boundsMatrixCountID;

        // Voxel Debug uniforms
        GLuint vd_projectMatID;

        // Post processing uniforms
        GLuint pp_framebufferTextureID;

        // Texture IDs
        GLuint framebufferTextureID;

        // Timestamps
        long long pointShaderTimestamp;
        long long boundsShaderTimestamp;
};


/// ##############################################################################################
///                                     OpenGl Globals
/// ##############################################################################################
static GLContext glContext;
static const uint32_t emptyAOGrid[AO_GRID_CELL_COUNT] = {};


/// ##############################################################################################
///                                     OpenGL Functions
/// ##############################################################################################
static void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                       GLsizei length, const GLchar* message, const void* user)
{
        if(severity == GL_DEBUG_SEVERITY_LOW || 
        severity == GL_DEBUG_SEVERITY_MEDIUM ||
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

GLuint glCreateShaderProgram(BumpAllocator* transientStorage, const char* shaderName)
{
        GLuint vertShaderID = glShaderInit(GL_VERTEX_SHADER,   
                (std::string("assets/shaders/") + shaderName + std::string(".vert")).c_str(), 
                transientStorage);
        GLuint fragShaderID = glShaderInit(GL_FRAGMENT_SHADER, 
                (std::string("assets/shaders/") + shaderName + std::string(".frag")).c_str(), 
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
                SM_ASSERT(false, "Failed to link point cloud shader: %s", programInfoLog);
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
        glContext.voxelProgramID      = glCreateShaderProgram (transientStorage, VOXEL_PROGRAM_NAME);
        glContext.boundsProgramID     = glCreateComputeProgram(transientStorage, BOUND_PROGRAM_NAME);
        glContext.fullscreenProgramID = glCreateShaderProgram (transientStorage, FULLSCREEN_PROGRAM_NAME);

        if(!glContext.pointProgramID   || 
           !glContext.computeProgramID ||
           !glContext.voxelProgramID   ||
           !glContext.boundsProgramID  ||
           !glContext.fullscreenProgramID) return false;

        glContext.pointShaderTimestamp = max
                                        (
                                                max(getTimestamp("assets/shaders/IFS.comp"),
                                                    getTimestamp("assets/shaders/voxel_cube.frag")),
                                                max(getTimestamp("assets/shaders/point_cloud.vert"),
                                                    getTimestamp("assets/shaders/point_cloud.frag"))
                                        );
        glContext.boundsShaderTimestamp = getTimestamp("assets/shaders/bounds_compute.comp");


        glGenVertexArrays(1, &glContext.pointVertexArrayID);
        glBindVertexArray(glContext.pointVertexArrayID);


        
        // SUPPLY POINT BUFFER TO COMPUTE SHADER TO WORK ON
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

        // SUPPLY TRANSFORM MATRICES TO COMPUTE SHADER
        {
                glGenBuffers(1, &glContext.matrixBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.matrixBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(mat4) * IFS::maxNumOfMatrix,
                             nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glContext.matrixBufferID);
        }

        // SUPPLY AMBIENT OCCLUSION GRID FOR LIGHTING
        {
                glGenBuffers(1, &glContext.AOgridBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.AOgridBufferID);
                glBufferData(GL_SHADER_STORAGE_BUFFER,
                             sizeof(emptyAOGrid), emptyAOGrid, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glContext.AOgridBufferID);
        }

        // SUPPLY AABB BOUNDS BUFFER 
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

        // SUPPLY FRAMEBUFFER OBJECT
        {
                glGenFramebuffers(1, &glContext.FBOID);
                glBindFramebuffer(GL_FRAMEBUFFER, glContext.FBOID);

                glGenTextures(1, & glContext.framebufferTextureID);
                glBindTexture(GL_TEXTURE_2D, glContext.framebufferTextureID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glContext.framebufferTextureID, 0);

                glGenRenderbuffers(1, &glContext.RBOID);
                glBindRenderbuffer(GL_RENDERBUFFER, glContext.RBOID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, RENDER_WIDTH, RENDER_HEIGHT);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glContext.RBOID);

                GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
                SM_ASSERT(framebufferStatus == GL_FRAMEBUFFER_COMPLETE,
                          "Framebuffer ERROR: status 0x%X", framebufferStatus);
        }


        // Compute uniforms
        glContext.computeTimeID = glGetUniformLocation(glContext.computeProgramID, "TIME");
        glContext.computePointCountID = glGetUniformLocation(glContext.computeProgramID, "pointCount");
        glContext.computeMatrixCountID = glGetUniformLocation(glContext.computeProgramID, "matrixCount");
        glContext.computeAOgridSizeID = glGetUniformLocation(glContext.computeProgramID, "AOgridSize");

        // Point cloud uniforms
        glContext.projectionID = glGetUniformLocation(glContext.pointProgramID, "cameraSpaceTransform");
        glContext.vertexAOgridSizeID = glGetUniformLocation(glContext.pointProgramID, "AOgridSize");

        // Bounds compute uniforms
        glContext.boundsMatrixCountID = glGetUniformLocation(glContext.boundsProgramID, "matrixCount");

        // Voxel debug uniforms
        glContext.vd_projectMatID = glGetUniformLocation(glContext.voxelProgramID, "projectionMatrix");

        // Post processing uniforms
        glContext.pp_framebufferTextureID = glGetUniformLocation(glContext.fullscreenProgramID, "screenTex");


        glEnable(GL_FRAMEBUFFER_SRGB);
        // glEnable(GL_PROGRAM_POINT_SIZE);

        return true;
}

void glRender(BumpAllocator* transientStorage, IFS& ifs, float deltaTime, float timeSeconds)
{
        // HOT SHADER RELOADING
        {
                long long timestampVert = getTimestamp("assets/shaders/point_cloud.vert");
                long long timestampFrag = getTimestamp("assets/shaders/point_cloud.frag");
                long long timestampCompute = getTimestamp("assets/shaders/IFS.comp");
                long long timestampBounds = getTimestamp("assets/shaders/bounds_compute.comp");

                if(timestampVert > glContext.pointShaderTimestamp ||
                timestampFrag > glContext.pointShaderTimestamp ||
                timestampCompute > glContext.pointShaderTimestamp)
                {
                        GLuint programID = glCreateShaderProgram(transientStorage, POINT_PROGRAM_NAME);
                        GLuint computeProgramID = glCreateComputeProgram(transientStorage, IFS_PROGRAM_NAME);

                        if(programID && computeProgramID)
                        {
                                glDeleteProgram(glContext.pointProgramID);
                                glDeleteProgram(glContext.computeProgramID);

                                glContext.pointProgramID = programID;
                                glContext.computeProgramID = computeProgramID;
                                glContext.projectionID = glGetUniformLocation(
                                        glContext.pointProgramID, "cameraSpaceTransform");
                                glContext.vertexAOgridSizeID = glGetUniformLocation(
                                        glContext.pointProgramID, "AOgridSize");
                                glContext.computeTimeID = glGetUniformLocation(
                                        glContext.computeProgramID, "TIME");
                                glContext.computePointCountID = glGetUniformLocation(
                                        glContext.computeProgramID, "pointCount");
                                glContext.computeMatrixCountID = glGetUniformLocation(
                                        glContext.computeProgramID, "matrixCount");
                                glContext.computeAOgridSizeID = glGetUniformLocation(
                                        glContext.computeProgramID, "AOgridSize");
                                glContext.pointShaderTimestamp = max(
                                        max(timestampVert, timestampFrag),
                                        timestampCompute);
                        }
                }

                if(timestampBounds > glContext.boundsShaderTimestamp)
                {
                        GLuint boundsProgramID = glCreateComputeProgram(
                                transientStorage, BOUND_PROGRAM_NAME);
                        if(boundsProgramID)
                        {
                                glDeleteProgram(glContext.boundsProgramID);
                                glContext.boundsProgramID = boundsProgramID;
                                glContext.boundsMatrixCountID = glGetUniformLocation(
                                        glContext.boundsProgramID, "matrixCount");
                                glContext.boundsShaderTimestamp = timestampBounds;
                        }
                }
                
        }

        
        if(renderData->pointCount < 0 || renderData->pointCount > MAX_POINTS)
        {
                SM_ASSERT(false, "Point count %d exceeds the render capacity of %d",
                        renderData->pointCount, MAX_POINTS);
                return;
        }

        // Update matricies
        ifs.generateTransformMatrix(deltaTime);

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

        // Compute point cloud positions
        {        
                glUseProgram(glContext.computeProgramID);
                glUniform1i(glContext.computePointCountID, renderData->pointCount);
                glUniform1f(glContext.computeTimeID, timeSeconds);
                glUniform1i(glContext.computeMatrixCountID, ifs.currNumOfMatrix);
                glUniform1i(glContext.computeAOgridSizeID, AO_GRID_SIDE_SIZE);

                GLuint workGroupCount = (renderData->pointCount + POINT_COMPUTE_LOCAL_SIZE - 1) /
                                                        POINT_COMPUTE_LOCAL_SIZE;

                glDispatchCompute(workGroupCount, 1, 1);

                glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }

        // Draw Debug voxel grid
        {
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glUseProgram(glContext.voxelProgramID);

                glUniformMatrix4fv(glContext.vd_projectMatID, 1, GL_FALSE, &cameraSpaceTransform.ax);
                //glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Use point cloud program to draw computed points
        {        
                glUseProgram(glContext.pointProgramID);
                
                glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &cameraSpaceTransform.ax);
                glUniform1i(glContext.vertexAOgridSizeID, AO_GRID_SIDE_SIZE);

                glBindVertexArray(glContext.pointVertexArrayID);
                glDrawArrays(GL_POINTS, 0, renderData->pointCount);
                glDrawArrays(GL_LINES, renderData->pointCount, BOUNDS_DEBUG_VERTEX_COUNT);
        }

        // Use fullscreen program to draw framebuffer texture to screen
        {
                glUseProgram(glContext.fullscreenProgramID);
                glUniform1i(glContext.pp_framebufferTextureID, 0);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDisable(GL_DEPTH_TEST);
                glBindTexture(GL_TEXTURE_2D, glContext.framebufferTextureID);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
}