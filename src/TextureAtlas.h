//
//  TextureAtlas.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 29.10.24.
//

#pragma once

#include "Quake3Types.h"
#include <string>
#include <vector>

struct TextureTile
{
    int x, y;
    int width, height;
};

struct TextureAtlas
{
    ~TextureAtlas();
    
    void initFromQ3Lightmaps(tBSPLightmap* lightmaps, int lenght);
    void saveToPng(const std::string &filename);
    
    std::vector<TextureTile> tiles;
    int width, height;
    
    unsigned char* buffer;
};
