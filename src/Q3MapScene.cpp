//
//  Q3MapScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#include "Q3MapScene.h"

#include "KeyValueCollection.h"
#include "Quake3Bsp.h"
#include "Camera.h"
#include "Player.h"
#include "PlayerMovement.h"

#include <glad/glad.h>

#include "miniaudio.h"
#include "ma_reverb_node.h"

#include "StudioRenderer.h"
#include "Q3LightGrid.h"

#include "Monster.h"

#include "Input.h"

#include "DebugRenderer.h"
#include "Cube.h"


#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

static ma_engine g_engine;
static ma_sound g_sound1;

static int prev_step_id = -1;
static bool is_right_step = true;
static ma_sound g_default_sound_steps[4];
static ma_sound g_wood_sound_steps[4];
static ma_sound g_snow_sound_steps[4];
static ma_sound g_clank_sound_steps[4];

static ma_lpf_node g_lpfNode;
static ma_reverb_node g_reverbNode;
static ma_delay_node g_delayNode;

static WiredCube cube;

static float step_timer = 0.0f;

std::vector<tBSPTexture> textures;

tBSPVisData m_clusters;

struct SoundEntity
{
    ma_sound sound;
    
    std::string filename;
    glm::vec3 origin;
    
    int cluster;
    bool isVisible = false;
};

static std::vector<SoundEntity> g_ambients;

Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
{
    studio = std::make_unique<StudioRenderer>();
    m_pLightGrid = std::make_unique<Q3LightGrid>();
    
    ma_engine_init(NULL, &g_engine);
    
    loadMap("assets/wolf/maps/escape2.bsp");
    
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_gun3.wav", 0, NULL, NULL, &g_sound1);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step1.wav", 0, NULL, NULL, &g_default_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step2.wav", 0, NULL, NULL, &g_default_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step3.wav", 0, NULL, NULL, &g_default_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step4.wav", 0, NULL, NULL, &g_default_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood1.wav", 0, NULL, NULL, &g_wood_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood2.wav", 0, NULL, NULL, &g_wood_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood3.wav", 0, NULL, NULL, &g_wood_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood4.wav", 0, NULL, NULL, &g_wood_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow1.wav", 0, NULL, NULL, &g_snow_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow2.wav", 0, NULL, NULL, &g_snow_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow3.wav", 0, NULL, NULL, &g_snow_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow4.wav", 0, NULL, NULL, &g_snow_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank1.wav", 0, NULL, NULL, &g_clank_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank2.wav", 0, NULL, NULL, &g_clank_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank3.wav", 0, NULL, NULL, &g_clank_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank4.wav", 0, NULL, NULL, &g_clank_sound_steps[3]);
    
    for (int i = 0; i < 4; ++i)
    {
        ma_sound_set_spatialization_enabled(&g_default_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_wood_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_snow_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_clank_sound_steps[i], false);
    }
    
    cube.init();
    
    for (auto& entity : g_ambients)
    {
        ma_sound_init_from_file(&g_engine, entity.filename.c_str(), 0, NULL, NULL, &entity.sound);
        
        ma_sound_set_spatialization_enabled(&entity.sound, true);
        ma_sound_set_positioning(&entity.sound, ma_positioning_absolute);
        ma_sound_set_attenuation_model(&entity.sound, ma_attenuation_model_linear);
        
        ma_sound_set_rolloff(&entity.sound, 1.0f);
        ma_sound_set_min_distance(&entity.sound, 80.0f);
        ma_sound_set_max_distance(&entity.sound, 1250.0f);
        
        ma_sound_set_position(&entity.sound, entity.origin.x, entity.origin.y, entity.origin.z);
        ma_sound_set_looping(&entity.sound, true);
        
        entity.cluster = m_collision.findCluster(entity.origin);
    }
}

Q3MapScene::~Q3MapScene() = default;

void Q3MapScene::loadMap(const std::string &filename)
{
    Quake3BSP bsp;

    if (!bsp.initFromFile(filename.c_str())) {
        return;
    }
    
    m_collision.initFromBsp(&bsp);
    
    m_pMovement = std::make_unique<PlayerMovement>(&m_collision);
    
    m_pPlayer = std::make_unique<Player>(m_pMovement.get());
    
    KeyValueCollection entities;
    entities.initFromString(bsp.m_entities);
    
    auto info_player_deathmatch = entities.getAllWithKeyValue("classname", "info_player_deathmatch");
    auto info_player_start = entities.getAllWithKeyValue("classname", "info_player_start");
    auto ai_soldier = entities.getAllWithKeyValue("classname", "ai_soldier");
    
    auto spawnPoints = info_player_deathmatch;
    spawnPoints.insert(spawnPoints.end(), info_player_start.begin(), info_player_start.end());
    spawnPoints.insert(spawnPoints.end(), ai_soldier.begin(), ai_soldier.end());
    
    for (int i = 0; i < spawnPoints.size(); ++i)
    {
        auto spawnPoint = spawnPoints[i];
        
        float yaw = 0;
        glm::vec3 position = {0, 0, 0};
        
        int angle = 0;
        
        if (spawnPoint.getIntValue("angle", angle))
        {
            yaw = radians(angle);
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            printf("entity_%i: origin = (%.0f, %.0f, %.0f)\n", i, origin.x, origin.y, origin.z);
            position = origin;
        }
        
        if (i == 0)
        {
            m_pPlayer->m_pModelInstance = studio->makeModelInstance("assets/models/v_9mmhandgun.mdl");
            m_pPlayer->position = position + glm::vec3{0, 0, 0.25};
            m_pPlayer->yaw = yaw;
            m_pPlayer->pitch = 0;
            
            m_pPlayer->m_pModelInstance->animator.setSeqIndex(2);
        }
        else
        {
            auto& monster = m_monsters.emplace_back();
            monster.m_pModelInstance = studio->makeModelInstance("assets/models/barney.mdl");
            monster.position = position;
            monster.yaw = yaw;
            
            int seqIndex = (int) (m_monsters.size() - 1) % monster.m_pModelInstance->animator.getNumSeq();
            monster.m_pModelInstance->animator.setSeqIndex(seqIndex);
        }
    }
    
    auto target_speaker = entities.getAllWithKeyValue("classname", "target_speaker");
    
    for (int i = 0; i < target_speaker.size(); ++i)
    {
        auto& speaker = target_speaker[i];
        
        int spawnflags = -1;
        speaker.getIntValue("spawnflags", spawnflags);
        
        if (spawnflags != 1) continue;
        
        glm::vec3 origin = {0, 0, 0};
        if (speaker.getVec3Value("origin", origin) == false) continue;
        
        if (speaker.properties.contains("noise") == false) continue;
        
        SoundEntity& entity = g_ambients.emplace_back();
        
        entity.filename = "assets/wolf/" + speaker.properties["noise"];
        entity.origin = origin;
    }
    
    auto func_door_rotating = entities.getAllWithKeyValue("classname", "func_door_rotating");
    
    for (auto& desc : func_door_rotating)
    {
        glm::vec3 origin = {0, 0, 0};
        if (desc.getVec3Value("origin", origin) == false) continue;
        
        if (desc.properties.contains("model") == false) continue;
        
        std::string& model = desc.properties["model"]; // it looks like "*23"
        
        int id = std::stoi(model.substr(1));
        
//        printf("func_door_rotating: model = %i \n", id);
        
        int faceIndex = bsp.m_models[id].faceIndex;
        int numOfFaces = bsp.m_models[id].numOfFaces;
        
        std::unordered_map<int, bool> processedVertices;
        
        for (int i = faceIndex; i < faceIndex + numOfFaces; ++i)
        {
            const tBSPFace &face = bsp.m_faces[i];
            
            for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
            {
                unsigned int index = bsp.m_indices[j] + face.startVertIndex;
                
                if (processedVertices[index]) {
                    continue;
                }
                
                tBSPVertex& vertex = bsp.m_verts[index];
                vertex.vPosition += origin;
                
                processedVertices[index] = true;
            }
        }
    }
    
    m_mesh.initFromBsp(&bsp);
    
    m_pLightGrid->init(bsp);
    
    textures = bsp.m_textures;
    
    m_clusters = bsp.m_clusters;
}

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point);

void Q3MapScene::update(float dt)
{
    if (m_pCamera == nullptr) return;
    
    m_pPlayer->update(dt);
    studio->queueViewModel(m_pPlayer->m_pModelInstance.get());
    
    glm::vec3 camera_pos = m_pPlayer->position;
    camera_pos.z += 40;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
    
    {
        glm::vec3 start = m_pPlayer->position;
        start.z += 40;
        
        ma_engine_listener_set_position(&g_engine, 0, start.x, start.y, start.z);
        
        ma_engine_listener_set_direction(&g_engine, 0, m_pCamera->getForward().x, m_pCamera->getForward().y, m_pCamera->getForward().z);
        ma_engine_listener_set_world_up(&g_engine, 0, 0, 0, 1);
    }
    
    for (auto& monster : m_monsters)
    {
        monster.update(dt);
        studio->queue(monster.m_pModelInstance.get());
    }
    
    for (int i = 0; i < m_monsters.size(); ++i)
    {
        if (Input::isKeyPressed(48 + i))
        {
            auto& monster = m_monsters[i];
    
            glm::vec3 dirToPlayer = glm::normalize(m_pPlayer->position - monster.position);
            monster.yaw = atan2(dirToPlayer.y, dirToPlayer.x);
        }
    }
    
    if (Input::isLeftMouseButtonClicked())
    {
//        ma_sound_start(&g_sound1);
        
        glm::vec3 start = m_pPlayer->position;
        glm::vec3 end = start + m_pCamera->getForward() * 2048.0f;
        
        DebugRenderer::getInstance().addLine(start, end, glm::vec3(1), 10.0f);
        
        if (auto hitted = traceEntities(start, end))
        {
//            auto pos = hitted->position;
//            ma_sound_set_position(&g_sound2, pos.x, pos.y, pos.z);
//            ma_sound_start(&g_sound2);
        }
    }
    
    if (m_pMovement->isWalk())
    {
        if (step_timer == 0.0f)
        {
            int step_id = rand() % 4;
            
            if (step_id == prev_step_id)
            {
                step_id = (step_id + 1) % 4;
            }
            
            prev_step_id = step_id;
            
            #define SURF_WOOD 0x40000
            #define SURF_SNOW 0x400000
            #define SURF_METAL 0x1000
            
            int surfaceFlags = m_pMovement->getSurfaceFlags();
            
            float LO = 0.9f;
            float HI = 1.1f;
            float pitch = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
            
            if (surfaceFlags & SURF_WOOD)
            {
                ma_sound& sound = g_wood_sound_steps[step_id];
                
                ma_sound_set_pitch(&sound, pitch);
                ma_sound_set_pan(&sound, is_right_step ? 0.1 : -0.1);
                ma_sound_start(&sound);
            }
            else if (surfaceFlags & SURF_SNOW)
            {
                ma_sound& sound = g_snow_sound_steps[step_id];
                
                ma_sound_set_pitch(&sound, pitch);
                ma_sound_set_pan(&sound, is_right_step ? 0.1 : -0.1);
                ma_sound_start(&sound);
            }
            else if (surfaceFlags & SURF_METAL)
            {
                ma_sound& sound = g_clank_sound_steps[step_id];
                
                ma_sound_set_pitch(&sound, pitch);
                ma_sound_set_pan(&sound, is_right_step ? 0.1 : -0.1);
                ma_sound_start(&sound);
            }
            else
            {
                ma_sound& sound = g_default_sound_steps[step_id];
                
                ma_sound_set_pitch(&sound, pitch);
                ma_sound_set_pan(&sound, is_right_step ? 0.1 : -0.1);
                ma_sound_start(&sound);
            }
            
            is_right_step = !is_right_step;
        }
        
        step_timer += dt;
        
        if (step_timer > 0.35f)
        {
            step_timer = 0.0f;
        }
    }
    else
    {
        step_timer = 0.0f;
        is_right_step = true;
    }
    
    DebugRenderer::getInstance().update(dt);
    
    int playerCluster = m_collision.findCluster(m_pPlayer->position);
    
    for (auto& entity : g_ambients)
    {
        int i = (playerCluster * m_clusters.bytesPerCluster) + (entity.cluster >> 3);
        byte visSet = m_clusters.pBitsets[i];
        
        bool isVisible = (visSet & (1 << (entity.cluster & 7))) != 0;
        
        if (entity.isVisible != isVisible)
        {
            isVisible ? ma_sound_start(&entity.sound) : ma_sound_stop(&entity.sound);
            entity.isVisible = isVisible;
        }
    }
}

void Q3MapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
    
    studio->drawRegular(m_pCamera, m_pLightGrid.get());
    
    glm::vec3 mins = glm::vec3{-15, -15, -24};
    glm::vec3 maxs = glm::vec3{ 15,  15,  48};
    
    for (auto& monster : m_monsters)
    {
        glm::vec3 absmins = monster.position + mins;
        glm::vec3 absmaxs = monster.position + maxs;
        
        cube.position = (absmins + absmaxs) * 0.5f;
        cube.scale = absmaxs - absmins;
        cube.color = {1, 1, 1, 1};
        
        cube.draw(*m_pCamera);
    }
    
    DebugRenderer::getInstance().draw(*m_pCamera);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    studio->drawViewModels(m_pCamera, m_pLightGrid.get());
    
//    for (auto& entity : g_ambients)
//    {
//        if (!entity.isVisible) continue;
//        
//        cube.position = entity.origin;
//        cube.scale = {8, 8, 8};
//        cube.color = {1, 0, 0, 1};
//        
//        cube.draw(*m_pCamera);
//    }
}

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/norm.hpp>

Monster* Q3MapScene::traceEntities(const glm::vec3 &start, const glm::vec3 &end)
{
    HitResult result;
    m_collision.trace(result, start, end, glm::vec3(0), glm::vec3(0));
    
    float maxDist = glm::distance2(result.endpos, start);
    
    Monster* nearestMonster = nullptr;
    float nearestDist = maxDist;
    
    for (auto& monster : m_monsters)
    {
        glm::vec3 mins = monster.position + glm::vec3{-15, -15, -24};
        glm::vec3 maxs = monster.position + glm::vec3{ 15,  15,  48};
        
        glm::vec3 point;
        bool check = intersection(start, end, mins, maxs, point);
        
        if (check)
        {
            float dist = glm::distance2(point, start);
            
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestMonster = &monster;
            }
        }
    }
    
    return nearestMonster;
}


#include <algorithm>
using std::min, std::max, std::swap;

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point)
{
    glm::vec3 dir = end - start;
    
    glm::vec3 t1 = (mins - start) / dir;
    glm::vec3 t2 = (maxs - start) / dir;

    // Ensure tmin and tmax are sorted
    if (t1.x > t2.x) { swap(t1.x, t2.x); }
    if (t1.y > t2.y) { swap(t1.y, t2.y); }
    if (t1.z > t2.z) { swap(t1.z, t2.z); }

    float tmin = max(max(t1.x, t1.y), t1.z);
    float tmax = min(min(t2.x, t2.y), t2.z);

    if (tmax > tmin && tmax > 0.0)
    {
        point = start + dir * tmin;
        return true;
    }
    
    return false;
}

std::vector<glm::vec4> generateUniqueColors(size_t colorCount)
{
    std::vector<glm::vec4> colors;
    
    if (colorCount == 0) {
        return colors;
    }
    
    // Используем равномерное распределение в цветовом пространстве HSV
    for (size_t i = 0; i < colorCount; ++i) {
        float hue = static_cast<float>(i) / static_cast<float>(colorCount); // Оттенок (0.0 - 1.0)
        float saturation = 0.7f; // Насыщенность
        float value = 0.9f;      // Яркость
        
        // Преобразуем HSV в RGB
        float c = value * saturation;
        float x = c * (1.0f - std::fabs(fmod(hue * 6.0f, 2.0f) - 1.0f));
        float m = value - c;
        
        glm::vec4 color;
        if (hue < 1.0f / 6.0f) {
            color = glm::vec4(c + m, x + m, m, 1.0f);
        } else if (hue < 2.0f / 6.0f) {
            color = glm::vec4(x + m, c + m, m, 1.0f);
        } else if (hue < 3.0f / 6.0f) {
            color = glm::vec4(m, c + m, x + m, 1.0f);
        } else if (hue < 4.0f / 6.0f) {
            color = glm::vec4(m, x + m, c + m, 1.0f);
        } else if (hue < 5.0f / 6.0f) {
            color = glm::vec4(x + m, m, c + m, 1.0f);
        } else {
            color = glm::vec4(c + m, m, x + m, 1.0f);
        }
        
        colors.push_back(color);
    }
    
    return colors;
}
