#pragma once

/// ##############################################################################################
///				        Platform Globals
/// ##############################################################################################
static bool running = true;
static KeyCodeID KeyCodeLookupTable[KEY_COUNT];


/// ##############################################################################################
///				        Platform Functions
/// ##############################################################################################
bool platformCreateWindow(int width, int height, const wchar_t* title);
void platformUpdateWindow();
void* platformLoadGlFunction(char* name);
void platformSwapBuffers();

void* platformLoadDynamicLibrary(char* dll);
void* platformLoadDynamicFunction(void* dll, char* funcName);
bool platformFreeDynamicLibrary(void* dll);

void platformFillKeycodeLookup();
