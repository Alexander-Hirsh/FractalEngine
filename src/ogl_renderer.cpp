#include "ogl_renderer.h"

#include "render_interface.h"


/// ##############################################################################################
///                                     OpenGL Structs
/// ##############################################################################################
struct GLContext
{
        GLuint pointProgramID;
        GLuint computeProgramID;

        GLuint pointVertexArrayID;
        GLuint pointBufferID;
        GLuint matrixBufferID;
        GLuint AOgridBufferID;

        GLuint projectionID;

        GLuint vertexAOgridSizeID;

        GLuint computeTimeID;
        GLuint computePointCountID;
        GLuint computeMatrixCountID;
        GLuint computeAOgridSizeID;
        long long pointShaderTimestamp;
};


/// ##############################################################################################
///                                     OpenGl Globals
/// ##############################################################################################
static GLContext glContext;


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



GLuint glShaderInit(int shaderType, char* shaderPath, BumpAllocator* transientStorage)
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

GLuint glCreatePointProgram(BumpAllocator* transientStorage)
{
        GLuint vertShaderID = glShaderInit(GL_VERTEX_SHADER,   "assets/shaders/point_cloud.vert", transientStorage);
        GLuint fragShaderID = glShaderInit(GL_FRAGMENT_SHADER, "assets/shaders/point_cloud.frag", transientStorage);

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

GLuint glCreateComputeProgram(BumpAllocator* transientStorage)
{
        GLuint compShaderID = glShaderInit(GL_COMPUTE_SHADER, "assets/shaders/IFS.comp", transientStorage);

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

        glContext.pointProgramID = glCreatePointProgram(transientStorage);
        glContext.computeProgramID = glCreateComputeProgram(transientStorage);

        if(!glContext.pointProgramID || !glContext.computeProgramID) return false;
        glContext.pointShaderTimestamp = max
                                        (
                                                getTimestamp("assets/shaders/IFS.comp"),
                                                max(getTimestamp("assets/shaders/point_cloud.vert"),
                                                    getTimestamp("assets/shaders/point_cloud.frag"))
                                        );


        glGenVertexArrays(1, &glContext.pointVertexArrayID);
        glBindVertexArray(glContext.pointVertexArrayID);

        // SUPPLY POINT BUFFER TO COMPUTE SHADER TO WORK ON
        {
                glGenBuffers(1, &glContext.pointBufferID);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.pointBufferID);

                vec4* initialPoints = (vec4*)bumpAlloc(transientStorage, sizeof(vec4) * MAX_POINTS);
                for(int i = 0; i < MAX_POINTS; i++)
                {
                        initialPoints[i] = {0.0f, 0.0f, 0.0f, 1.0f};
                }

                glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * MAX_POINTS,
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
                             sizeof(bool) * AO_GRID_SIDE_SIZE * AO_GRID_SIDE_SIZE * AO_GRID_SIDE_SIZE,
                             nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glContext.AOgridBufferID);
        }

        glContext.projectionID = glGetUniformLocation(glContext.pointProgramID, "cameraSpaceTransform");
        glContext.vertexAOgridSizeID = glGetUniformLocation(glContext.pointProgramID, "AOgridSize");

        glContext.computeTimeID = glGetUniformLocation(glContext.computeProgramID, "TIME");
        glContext.computePointCountID = glGetUniformLocation(glContext.computeProgramID, "pointCount");
        glContext.computeMatrixCountID = glGetUniformLocation(glContext.computeProgramID, "matrixCount");
        glContext.computeAOgridSizeID = glGetUniformLocation(glContext.computeProgramID, "AOgridSize");


        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);

        return true;
}

void glRender(BumpAllocator* transientStorage, IFS& ifs, float deltaTime, float timeSeconds)
{
        // HOT SHADER RELOADING
        {
                long long timestampVert = getTimestamp("assets/shaders/point_cloud.vert");
                long long timestampFrag = getTimestamp("assets/shaders/point_cloud.frag");
                long long timestampCompute = getTimestamp("assets/shaders/IFS.comp");

                if(timestampVert > glContext.pointShaderTimestamp ||
                timestampFrag > glContext.pointShaderTimestamp ||
                timestampCompute > glContext.pointShaderTimestamp)
                {
                        GLuint programID = glCreatePointProgram(transientStorage);
                        GLuint computeProgramID = glCreateComputeProgram(transientStorage);

                        if(programID && computeProgramID)
                        {
                                glDeleteProgram(glContext.pointProgramID);
                                glDeleteProgram(glContext.computeProgramID);

                                glContext.pointProgramID = programID;
                                glContext.computeProgramID = computeProgramID;
                                glContext.projectionID = glGetUniformLocation(programID, "cameraSpaceTransform");
                                glContext.computePointCountID = glGetUniformLocation(computeProgramID, "pointCount");
                                glContext.computeTimeID = glGetUniformLocation(computeProgramID, "TIME");
                                glContext.computeMatrixCountID = glGetUniformLocation(computeProgramID, "matrixCount");
                                glContext.pointShaderTimestamp = max(timestampCompute,
                                                                max(timestampVert, timestampFrag));
                        }
                        else
                        {
                                if(programID) glDeleteProgram(programID);
                                if(computeProgramID) glDeleteProgram(computeProgramID);
                        }
                }
                
        }

        
        if(renderData->pointCount < 0 || renderData->pointCount > MAX_POINTS)
        {
                SM_ASSERT(false, "Point count %d exceeds the render capacity of %d",
                        renderData->pointCount, MAX_POINTS);
                return;
        }

        ifs.generateTransformMatrix(deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, input->screenSize.x, input->screenSize.y);


        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.pointBufferID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, glContext.matrixBufferID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, glContext.AOgridBufferID);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glContext.matrixBufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                        (GLsizeiptr)(sizeof(mat4) * ifs.currNumOfMatrix),
                        ifs.IFSMatrices);

        glUseProgram(glContext.computeProgramID);
        glUniform1i(glContext.computePointCountID, renderData->pointCount);
        glUniform1f(glContext.computeTimeID, timeSeconds);
        glUniform1i(glContext.computeMatrixCountID, ifs.currNumOfMatrix);

        GLuint workGroupCount = (renderData->pointCount + POINT_COMPUTE_LOCAL_SIZE - 1) /
                                                POINT_COMPUTE_LOCAL_SIZE;

        glDispatchCompute(workGroupCount, 1, 1);

        //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);  // Do I even need this?

        // Use base vertex + fragment shader to draw computed points
        {        
                glUseProgram(glContext.pointProgramID);

                Camera3D& camera = renderData->gameCamera;
                float top = camera.nearPlane * tanf(degToRad(camera.fov / 2.0f));
                float right = top * ((float)input->screenSize.x / input->screenSize.y);

                camera.position.z = 2.0f;
                mat4 projection = perspectiveProjection(-right, right, top, -top, camera.nearPlane, camera.farPlane);
                mat4 view = translationMatrix(camera.position * -1.0f);

                mat4 cameraSpaceTransform = projection * view;

                glUniformMatrix4fv(glContext.projectionID, 1, GL_FALSE, &cameraSpaceTransform.ax);
                glUniform1i(glContext.vertexAOgridSizeID, AO_GRID_SIDE_SIZE);

                glBindVertexArray(glContext.pointVertexArrayID);
                glDrawArrays(GL_POINTS, 0, renderData->pointCount);
        }
}