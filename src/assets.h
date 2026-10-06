#pragma once

#include "math_lib.h"


/// ##############################################################################################
///                                     Assets Constants
/// ##############################################################################################


/// ##############################################################################################
///                                     Assets Structs
/// ##############################################################################################
enum SpriteID
{
        SPRITE_DUDE,
        SPRITE_PLANET,

        SPRITE_COUNT
};

struct Sprite
{
  ivec2 atlasOffset;
  ivec2 size;
  int frameCount = 1;
};

/// ##############################################################################################
///                                     Assets Functions
/// ##############################################################################################
Sprite getSprite(SpriteID spriteID)
{
        Sprite sprite = {};
        sprite.frameCount = 1;

        switch(spriteID)
        {
                case SPRITE_DUDE:
                {
                        sprite.atlasOffset = {0, 0};
                        sprite.size = {6, 11};
                        break;
                }

                case SPRITE_PLANET:
                {
                        sprite.atlasOffset = {6, 11};
                        sprite.size = {200, 200};
                        break;
                }
        }

        return sprite;
}