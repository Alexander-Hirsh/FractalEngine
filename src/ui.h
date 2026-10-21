#pragma once

/// ##############################################################################################
///                                     UI Structs
/// ##############################################################################################
enum uiElemType
{
        PLANE,
        TEXT,
        BUTTON,
        TEXT_BOX,
        TICK_BOX,
        SLIDER,
        SLIDER_HANDLE
};

struct uiElement
{
    vec2 pos;
    vec2 bound;
    uiElemType type;

    bool active;
    bool hot;
};