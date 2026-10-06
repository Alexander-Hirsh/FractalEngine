#pragma once

#include <stdio.h>
#include <stdlib.h>     // For malloc
#include <string.h>     // For memset
#include <sys/stat.h>   // For timestamp
#include <math.h>

#include "rng.h"

/// ##############################################################################################
///                                     Defines
/// ##############################################################################################
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

#define bool8 char
#define BIT(x) 1 << (x)
#define KB(x) ((unsigned long long)1024 * x)
#define MB(x) ((unsigned long long)1024 * KB(x))
#define GB(x) ((unsigned long long)1024 * MB(x))


/// ##############################################################################################
///                                     Logging
/// ##############################################################################################
enum TextColor
{  
        TEXT_COLOR_BLACK,
        TEXT_COLOR_RED,
        TEXT_COLOR_GREEN,
        TEXT_COLOR_YELLOW,
        TEXT_COLOR_BLUE,
        TEXT_COLOR_MAGENTA,
        TEXT_COLOR_CYAN,
        TEXT_COLOR_WHITE,
        TEXT_COLOR_BRIGHT_BLACK,
        TEXT_COLOR_BRIGHT_RED,
        TEXT_COLOR_BRIGHT_GREEN,
        TEXT_COLOR_BRIGHT_YELLOW,
        TEXT_COLOR_BRIGHT_BLUE,
        TEXT_COLOR_BRIGHT_MAGENTA,
        TEXT_COLOR_BRIGHT_CYAN,
        TEXT_COLOR_BRIGHT_WHITE,
        TEXT_COLOR_COUNT
};

template <typename ...Args>

void _log(char* prefix, char* msg, TextColor textColor, Args... args)
{
        static char* TextColorTable[TEXT_COLOR_COUNT] = 
        {    
                "\x1b[30m", // TEXT_COLOR_BLACK
                "\x1b[31m", // TEXT_COLOR_RED
                "\x1b[32m", // TEXT_COLOR_GREEN
                "\x1b[33m", // TEXT_COLOR_YELLOW
                "\x1b[34m", // TEXT_COLOR_BLUE
                "\x1b[35m", // TEXT_COLOR_MAGENTA
                "\x1b[36m", // TEXT_COLOR_CYAN
                "\x1b[37m", // TEXT_COLOR_WHITE
                "\x1b[90m", // TEXT_COLOR_BRIGHT_BLACK
                "\x1b[91m", // TEXT_COLOR_BRIGHT_RED
                "\x1b[92m", // TEXT_COLOR_BRIGHT_GREEN
                "\x1b[93m", // TEXT_COLOR_BRIGHT_YELLOW
                "\x1b[94m", // TEXT_COLOR_BRIGHT_BLUE
                "\x1b[95m", // TEXT_COLOR_BRIGHT_MAGENTA
                "\x1b[96m", // TEXT_COLOR_BRIGHT_CYAN
                "\x1b[97m", // TEXT_COLOR_BRIGHT_WHITE
        };

        char formatBuffer[8192] = {};
        sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[textColor], prefix, msg);

        char textBuffer[8912] = {};
        sprintf(textBuffer, formatBuffer, args...);

        puts(textBuffer);
}

#define SM_TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_GREEN,  ##__VA_ARGS__);
#define SM_WARNG(msg, ...) _log("WARNG: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define SM_ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED,    ##__VA_ARGS__);

#define SM_ASSERT(x, msg, ...)                  \
{                                               \
        if(!(x))                                \
        {                                       \
                SM_ERROR(msg, ##__VA_ARGS__);   \
                DEBUG_BREAK();                  \
                SM_ERROR("Assertion HIT!")      \
        }                                       \
}



/// ##############################################################################################
///                                     Bump Allocator
/// ##############################################################################################
struct BumpAllocator
{
        size_t capacity;
        size_t used;
        char* memory;
};

BumpAllocator makeBumpAllocator(size_t size)
{
        BumpAllocator ba = {};

        ba.memory = (char*)malloc(size);
        if(ba.memory)
        {
                ba.capacity = size;
                memset(ba.memory, 0, size); // Set all memory to 0
        }
        else
        {
                SM_ASSERT(false, "Failed to allocate enough memory")
        }

        ba.capacity = size; 

        return ba;
}

char* bumpAlloc(BumpAllocator* bumpAllocator, size_t size)
{
        char* result = nullptr;

        size_t allignedSize = (size + 7) & ~ 7; // Add padding
        if(bumpAllocator->used + allignedSize <= bumpAllocator->capacity)
        {
                result = bumpAllocator->memory + bumpAllocator->used;
                bumpAllocator->used += allignedSize;
        }
        else
        {
                SM_ASSERT(false, "BumpAllocator is full");
        }

        return result;
}


/// ##############################################################################################
///                                     File I/O
/// ##############################################################################################
long long max(long long a, long long b)
{
        if(a > b) return a;

        return b;
}

long long getTimestamp(const char* file)
{
        struct stat file_stat = {};
        if(stat(file, &file_stat) != 0)
        {
                return 0;
        }

        return file_stat.st_mtime;
}

bool fileExists(const char* filePath)
{
        SM_ASSERT(filePath, "No filePath supplied!");

        auto file = fopen(filePath, "rb");
        if(!file)
        {
                return false;
        }
        fclose(file);

        return true;
}

long getFileSize(const char* filePath)
{
        SM_ASSERT(filePath, "No filePath supplied!");

        long fileSize = 0;
        auto file = fopen(filePath, "rb");
        if(!file)
        {
                SM_ERROR("Failed opening File: %s", filePath);
                return 0;
        }

        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        fclose(file);

        return fileSize;
}

/*
* Reads a file into a supplied buffer. We manage our own
* memory and therefore want more control over where it 
* is allocated
*/
char* readFile(const char* filePath, int* fileSize, char* buffer)
{
        SM_ASSERT(filePath, "No filePath supplied!");
        SM_ASSERT(fileSize, "No fileSize supplied!");
        SM_ASSERT(buffer, "No buffer supplied!");

        *fileSize = 0;
        auto file = fopen(filePath, "rb");
        if(!file)
        {
                SM_ERROR("Failed opening File: %s", filePath);
                return nullptr;
        }

        fseek(file, 0, SEEK_END);
        *fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        memset(buffer, 0, *fileSize + 1);
        fread(buffer, sizeof(char), *fileSize, file);

        fclose(file);

        return buffer;
}

char* readFile(const char* filePath, int* fileSize, BumpAllocator* bumpAllocator)
{
        char* file = nullptr;
        long fileSize2 = getFileSize(filePath);

        if(fileSize2)
        {
                char* buffer = bumpAlloc(bumpAllocator, fileSize2 + 1);

                file = readFile(filePath, fileSize, buffer);
        }

        return file; 
}

void writeFile(const char* filePath, char* buffer, int size)
{
        SM_ASSERT(filePath, "No filePath supplied!");
        SM_ASSERT(buffer, "No buffer supplied!");
        auto file = fopen(filePath, "wb");
        if(!file)
        {
                SM_ERROR("Failed opening File: %s", filePath);
                return;
        }

        fwrite(buffer, sizeof(char), size, file);
        fclose(file);
}

bool copyFile(const char* fileName, const char* outputName, char* buffer)
{
        int fileSize = 0;
        char* data = readFile(fileName, &fileSize, buffer);

        auto outputFile = fopen(outputName, "wb");
        if(!outputFile)
        {
                SM_ERROR("Failed opening File: %s", outputName);
                return false;
        }

        int result = fwrite(data, sizeof(char), fileSize, outputFile);
        if(!result)
        {
                SM_ERROR("Failed opening File: %s", outputName);
                return false;
        }

        fclose(outputFile);

        return true;
}

bool copyFile(const char* fileName, const char* outputName, BumpAllocator* bumpAllocator)
{
        //char* file = 0;
        long fileSize2 = getFileSize(fileName);

        if(fileSize2)
        {
                char* buffer = bumpAlloc(bumpAllocator, fileSize2 + 1);

                return copyFile(fileName, outputName, buffer);
        }

        return false;
}


/// ##############################################################################################
///                                     Math
/// ##############################################################################################
struct vec2
{
        float x, y;

        vec2() : x(0), y(0) {}
        vec2(float x, float y) : x(x), y(y) {}

        vec2 operator+(const vec2& other) const
        {
                return { x + other.x, y + other.y };
        }

        vec2 operator*(float s) const
        {
                return { x * s, y * s };
        }

        vec2 operator*(const vec2& other) const
        {
                return { x * other.x, y * other.y };
        }

        vec2 operator/(float s) const
        {
                return { x / s, y / s };
        }

        vec2 operator/(vec2 other) const
        {
                return { x / other.x, y / other.y };
        }

        vec2 operator-(const vec2& other) const
        {
                return { x - other.x, y - other.y };
        }

        vec2& operator+=(vec2 v)
        {
                x += v.x;
                y += v.y;
                return *this;
        }
                
        vec2& operator-=(vec2 v)
        {
                x -= v.x;
                y -= v.y;
                return *this;
        }

        vec2& operator/=(float s)
        {
                x /= s;
                y /= s;
                return *this;
        }

        vec2& operator-=(float s)
        {
                x -= s;
                y -= s;
                return *this;
        }

        bool operator==(const vec2& other) const
        {
                return x == other.x && y == other.y;
        }

        bool operator>(const vec2& other) const
        {
                return x > other.x && y > other.y;
        }

        bool operator<(const vec2& other) const
        {
                return x < other.x && y < other.y;
        }

        bool operator>=(const vec2& other) const
        {
                return x >= other.x && y >= other.y;
        }

        bool operator<=(const vec2& other) const
        {
                return x <= other.x && y <= other.y;
        }
};

struct ivec2
{
        int x, y;

        ivec2() : x(0), y(0) {}
        ivec2(int x, int y) : x(x), y(y) {}

        ivec2 operator+(const ivec2& other) const
        {
                return { x + other.x, y + other.y };
        }

        ivec2 operator*(int s) const
        {
                return { x * s, y * s };
        }

        ivec2 operator*(const ivec2& other) const
        {
                return { x * other.x, y * other.y };
        }

        ivec2 operator/(int s) const
        {
                return { x / s, y / s };
        }

        ivec2 operator/(ivec2 other) const
        {
                return { x / other.x, y / other.y };
        }

        ivec2 operator-(const ivec2& other) const
        {
                return { x - other.x, y - other.y };
        }

        ivec2& operator+=(ivec2 v)
        {
                x += v.x;
                y += v.y;
                return *this;
        }
                
        ivec2& operator-=(ivec2 v)
        {
                x -= v.x;
                y -= v.y;
                return *this;
        }

        ivec2& operator/=(int s)
        {
                x /= s;
                y /= s;
                return *this;
        }

        ivec2& operator-=(int s)
        {
                x -= s;
                y -= s;
                return *this;
        }

        bool operator==(const ivec2& other) const
        {
                return x == other.x && y == other.y;
        }

        bool operator>(const ivec2& other) const
        {
                return x > other.x && y > other.y;
        }

        bool operator<(const ivec2& other) const
        {
                return x < other.x && y < other.y;
        }

        bool operator>=(const ivec2& other) const
        {
                return x >= other.x && y >= other.y;
        }

        bool operator<=(const ivec2& other) const
        {
                return x <= other.x && y <= other.y;
        }
};

vec2 Ivec2vec(ivec2 v)
{
        return vec2((float)v.x, (float)v.y);
}
ivec2 vec2Ivec(vec2 v)
{
        return ivec2((int)v.x, (int)v.y);
}

vec2 normalize(vec2 v)
{
        float len = sqrt(v.x * v.x + v.y * v.y);
        if (len > 0.0f) {
                return { v.x / len, v.y / len };
        }else return {0.0f, 0.0f};
}

struct vec3
{
        float x, y, z;

        vec3() : x(0), y(0), z(0) {}
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}

        vec3 operator+(const vec3& other) const
        {
                return { x + other.x, y + other.y, z + other.z };
        }

        vec3 operator+(float s) const
        {
                return { x + s, y + s, z + s };
        }

        vec3 operator-(const vec3& other) const
        {
                return { x - other.x, y - other.y, z - other.z };
        }

        vec3 operator-(float s) const
        {
                return { x - s, y - s, z - s };
        }

        vec3 operator*(float s) const
        {
                return { x * s, y * s, z * s };
        }

        vec3 operator/(float s) const
        {
                return { x / s, y / s, z / s };
        }

        vec3& operator+=(const vec3& other)
        {
                x += other.x;
                y += other.y;
                z += other.z;
                return *this;
        }

        vec3& operator-=(const vec3& other)
        {
                x -= other.x;
                y -= other.y;
                z -= other.z;
                return *this;
        }

        vec3& operator/=(float s)
        {
                x /= s;
                y /= s;
                z /= s;
                return *this;
        }

        vec3& operator*=(float s)
        {
                x *= s;
                y *= s;
                z *= s;
                return *this;
        }
};


struct vec4
{
        union
        {
                float values[4];
                struct
                {
                        float x;
                        float y;
                        float z;
                        float w;
                };

                struct
                {
                        float r;
                        float g;
                        float b;
                        float a;
                };
        };

        float& operator[](int idx)
        {
                return values[idx];
        }

        bool operator==(vec4 other)
        {
                return x == other.x && y == other.y && z == other.z && w == other.w;
        }
};

struct mat4
{
        union 
        {
                vec4 values[4];
                struct
                {
                float ax;
                float bx;
                float cx;
                float dx;

                float ay;
                float by;
                float cy;
                float dy;

                float az;
                float bz;
                float cz;
                float dz;

                float aw;
                float bw;
                float cw;
                float dw;
                };
        };

        vec4& operator[](int col)
        {
                return values[col];
        }

        mat4 operator*(const mat4& b) const
        {
                mat4 result = {};

                result.ax = ax * b.ax + ay * b.bx + az * b.cx + aw * b.dx;
                result.ay = ax * b.ay + ay * b.by + az * b.cy + aw * b.dy;
                result.az = ax * b.az + ay * b.bz + az * b.cz + aw * b.dz;
                result.aw = ax * b.aw + ay * b.bw + az * b.cw + aw * b.dw;

                result.bx = bx * b.ax + by * b.bx + bz * b.cx + bw * b.dx;
                result.by = bx * b.ay + by * b.by + bz * b.cy + bw * b.dy;
                result.bz = bx * b.az + by * b.bz + bz * b.cz + bw * b.dz;
                result.bw = bx * b.aw + by * b.bw + bz * b.cw + bw * b.dw;

                result.cx = cx * b.ax + cy * b.bx + cz * b.cx + cw * b.dx;
                result.cy = cx * b.ay + cy * b.by + cz * b.cy + cw * b.dy;
                result.cz = cx * b.az + cy * b.bz + cz * b.cz + cw * b.dz;
                result.cw = cx * b.aw + cy * b.bw + cz * b.cw + cw * b.dw;

                result.dx = dx * b.ax + dy * b.bx + dz * b.cx + dw * b.dx;
                result.dy = dx * b.ay + dy * b.by + dz * b.cy + dw * b.dy;
                result.dz = dx * b.az + dy * b.bz + dz * b.cz + dw * b.dz;
                result.dw = dx * b.aw + dy * b.bw + dz * b.cw + dw * b.dw;

        return result;
        }
};

mat4 orthographicProjection(float left, float right, float top, float bottom,
                            float rotation = 0.0f)
{
        mat4 result = {};

        float scaleX = 2.0f / (right - left);
        float scaleY = 2.0f / (top - bottom);
        float centerX = (right + left) / 2.0f;
        float centerY = (top + bottom) / 2.0f;
        float cosRotation = cos(rotation);
        float sinRotation = sin(rotation);

        // Apply the inverse camera rotation so the world moves opposite the camera.
        result[0][0] = scaleX * cosRotation;
        result[0][1] = scaleY * -sinRotation;
        result[1][0] = scaleX * sinRotation;
        result[1][1] = scaleY * cosRotation;
        result.aw = -scaleX * (centerX * cosRotation - centerY * sinRotation);
        result.bw = scaleY * (centerY * cosRotation + centerX * sinRotation);
        result.cw = 0.0f; // Near Plane
        result[2][2] = 1.0f / (1.0f - 0.0f); // Far and Near
        result[3][3] = 1.0f;

        return result;
}

mat4 perspectiveProjection(float left, float right, float top, float bottom, 
                           float nearPlane, float farPlane)
{
        mat4 result = {};

        result[0][0] = 2.0f * nearPlane / (right - left);
        result[1][1] = 2.0f * nearPlane / (top - bottom);

        result[2][0] = (right + left) / (right - left);
        result[2][1] = (top + bottom) / (top - bottom);

        result[2][2] = -(farPlane + nearPlane)
                        / (farPlane - nearPlane);

        result[2][3] = -1.0f;

        result[3][2] = -(2.0f * farPlane * nearPlane)
                        / (farPlane - nearPlane);

        return result;
}


float degToRad(float d)
{
        return d * 0.0174533f;
}

mat4 constrScaleMatrix(vec3 scale)
{
        mat4 result;
        result.ax = scale.x;
        result.by = scale.y;
        result.cz = scale.z;
        result.dw = 1;

        return result;
}

mat4 constrRotationMatrix(vec3 rot)
{
        mat4 result;
        float& x = rot.x;
        float& y = rot.y;
        float& z = rot.z;

        result.ax =  cos(y) * cos(z);
        result.ay = -cos(y) * sin(z);
        result.az =  sin(y);

        result.bx =  cos(x) * sin(z) + sin(x) * sin(y) * cos(z);
        result.by =  cos(x) * cos(z) - sin(x) * sin(y) * sin(z);
        result.bz = -sin(x) * cos(y);

        result.cx = sin(x) * sin(z) - cos(x) * sin(y) * cos(z);
        result.cy = sin(x) * cos(z) + cos(x) * sin(y) * sin(z); 
        result.cz = cos(x) * cos(y);

        result.dw = 1;

        return result;
}

mat4 constrShearMatrix(vec3 shear)
{
        mat4 result;

        float x = tan(shear.x);
        float y = tan(shear.y);
        float z = tan(shear.z);

        result.ax = 1.0f;
        result.ay = x;
        result.az = 0.0f;

        result.bx = 0.0f;
        result.by = 1.0f;
        result.bz = y;

        result.cx = z;
        result.cy = 0.0f;
        result.cz = 1.0f;

        result.dw = 1.0f;

        return result;
}

mat4 constrTranslationMatrix(vec3 translation)
{
        mat4 result;
        result.aw = translation.x;
        result.bw = translation.y;
        result.cw = translation.z;
        result.dw = 1;

        return result;
}

vec3 randVec3(float min, float max)
{
        float x = rng_f() * (max - min) + min;
        float y = rng_f() * (max - min) + min;
        float z = rng_f() * (max - min) + min;

        return vec3(x, y, z);
}