//
//  StudioRenderer.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#pragma once

#include <memory>

#include "Shader.h"
#include "GoldSrcModel.h"

class Camera;
struct Q3LightGrid;

struct StudioRenderer
{
    StudioRenderer();
    
    GoldSrcModelInstance* makeModelInstance(const std::string& filename);
    
    void update(float dt);
    void draw(Camera* camera);
    void drawWeapon(Camera* camera);
    
    Q3LightGrid* m_lightGrid = nullptr;
    
private:
    Shader m_shader;
    
    std::unordered_map<std::string, GoldSrcModel> m_cache;
    std::vector<GoldSrcModelInstance> m_instances;
    
    std::unique_ptr<GoldSrcModelInstance> m_player;
    
    GoldSrcModel* makeModel(const std::string& filename);
};
