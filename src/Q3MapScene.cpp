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
static ma_sound g_sound2;
static ma_sound g_sound3;

static int prev_step_id = -1;
static bool is_right_step = true;
static ma_sound g_sound_steps[4];

static ma_lpf_node g_lpfNode;
static ma_reverb_node g_reverbNode;

static WiredCube cube;

static float step_timer = 0.0f;

std::vector<tBSPTexture> textures;

Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
{
    studio = std::make_unique<StudioRenderer>();
    m_pLightGrid = std::make_unique<Q3LightGrid>();
    
    loadMap("assets/q3/maps/q3dm7.bsp");
    
    ma_engine_init(NULL, &g_engine);
    
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_gun3.wav", 0, NULL, NULL, &g_sound1);
    ma_sound_init_from_file(&g_engine, "assets/sounds/donthurtem.wav", 0, NULL, NULL, &g_sound2);
    ma_sound_init_from_file(&g_engine, "assets/sounds/leavealone.wav", 0, NULL, NULL, &g_sound3);
    
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_step1.wav", 0, NULL, NULL, &g_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_step2.wav", 0, NULL, NULL, &g_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_step3.wav", 0, NULL, NULL, &g_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/sounds/pl_step4.wav", 0, NULL, NULL, &g_sound_steps[3]);
    
    
    
    ma_sound_set_spatialization_enabled(&g_sound2, true);
    ma_sound_set_positioning(&g_sound2, ma_positioning_absolute);
    ma_sound_set_attenuation_model(&g_sound2, ma_attenuation_model_inverse);
    
    ma_sound_set_rolloff(&g_sound2, 0.95f);
    ma_sound_set_min_distance(&g_sound2, 256.0f);
    ma_sound_set_max_distance(&g_sound2, 2048.0f);
    
    ma_sound_set_spatialization_enabled(&g_sound1, false);
    ma_sound_set_spatialization_enabled(&g_sound3, false);
    
    ma_sound_set_spatialization_enabled(&g_sound_steps[0], false);
    ma_sound_set_spatialization_enabled(&g_sound_steps[1], false);
    ma_sound_set_spatialization_enabled(&g_sound_steps[2], false);
    ma_sound_set_spatialization_enabled(&g_sound_steps[3], false);
    
    {
        ma_uint32 channels   = ma_engine_get_channels(&g_engine);
        ma_uint32 sampleRate = ma_engine_get_sample_rate(&g_engine);
        
        ma_reverb_node_config config = ma_reverb_node_config_init(channels, sampleRate);
        
        ma_reverb_node_init(ma_engine_get_node_graph(&g_engine), &config, NULL, &g_reverbNode);
        ma_node_attach_output_bus(&g_reverbNode, 0, ma_engine_get_endpoint(&g_engine), 0);
        
        g_reverbNode.reverb.dry = 0.75;
        g_reverbNode.reverb.wet = 0.25;
        
        ma_node_attach_output_bus(&g_sound1, 0, &g_reverbNode, 0);
        
        ma_node_attach_output_bus(&g_sound_steps[0], 0, &g_reverbNode, 0);
        ma_node_attach_output_bus(&g_sound_steps[1], 0, &g_reverbNode, 0);
        ma_node_attach_output_bus(&g_sound_steps[2], 0, &g_reverbNode, 0);
        ma_node_attach_output_bus(&g_sound_steps[3], 0, &g_reverbNode, 0);
    }
    
    {
        ma_uint32 channels   = ma_engine_get_channels(&g_engine);
        ma_uint32 sampleRate = ma_engine_get_sample_rate(&g_engine);
        ma_lpf_node_config lpfNodeConfig = ma_lpf_node_config_init(channels, sampleRate, 500, 12);
        
        ma_lpf_node_init(ma_engine_get_node_graph(&g_engine), &lpfNodeConfig, NULL, &g_lpfNode);
        
        ma_node_attach_output_bus(&g_lpfNode, 0, ma_engine_get_endpoint(&g_engine), 0);
        
        ma_node_attach_output_bus(&g_sound3, 0, &g_lpfNode, 0);
    }
    
    cube.init();
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
    
    m_mesh.initFromBsp(&bsp);
    
    m_pLightGrid->init(bsp);
    
    textures = bsp.m_textures;
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
    
    if (Input::isRightMouseButtonClicked())
    {
        glm::vec3 start = m_pPlayer->position;
        start.z += 40;
        
        glm::vec3 end = m_monsters[5].position;
        
        HitResult result;
        m_collision.trace(result, start, end, glm::vec3(0), glm::vec3(0));
        
        DebugRenderer::getInstance().addLine(start, end, glm::vec3(1), 10.0f);
        
        float cutoff = (result.fraction == 1) ? 10000 : 3500;
        
        ma_uint32 channels   = ma_engine_get_channels(&g_engine);
        ma_uint32 sampleRate = ma_engine_get_sample_rate(&g_engine);
        
        ma_lpf_config lpfConfig = ma_lpf_config_init(ma_format_f32, channels, sampleRate, cutoff, 12);
        ma_lpf_reinit(&lpfConfig, &g_lpfNode.lpf);
        
        ma_sound_start(&g_sound3);
    }
    
    if (Input::isLeftMouseButtonClicked())
    {
        ma_sound_start(&g_sound1);
        
        glm::vec3 start = m_pPlayer->position;
        start.z += 40;
        
//        ma_engine_listener_set_position(&g_engine, 0, start.x, start.y, start.z);
//        
//        ma_engine_listener_set_direction(&g_engine, 0, m_pCamera->getForward().x, m_pCamera->getForward().y, m_pCamera->getForward().z);
//        ma_engine_listener_set_world_up(&g_engine, 0, 0, 0, 1);
        
        glm::vec3 end = start + m_pCamera->getForward() * 1024.0f;
        
        DebugRenderer::getInstance().addLine(start, end, glm::vec3(1), 10.0f);
        
        HitResult result;
        m_collision.trace(result, start, end, glm::vec3(0), glm::vec3(0));
        
        if (result.textureId != -1) {
            printf("texture = %s\n", textures[result.textureId].strName);
        }
        
        float dist1 = glm::length(result.endpos - start);
        
        for (const auto& monster : m_monsters)
        {
            glm::vec3 mins = monster.position + glm::vec3{-15, -15, -24};
            glm::vec3 maxs = monster.position + glm::vec3{ 15,  15,  48};
            
            glm::vec3 point;
            bool check = intersection(start, end, mins, maxs, point);
            float dist2 = glm::length(point - start);
            
            if (check && dist2 < dist1)
            {
                ma_sound_set_position(&g_sound2, monster.position.x, monster.position.y, monster.position.z);
                ma_sound_start(&g_sound2);
                break;
            }
        }
    }
    
    if (m_pMovement->isWalk())
    {
        if (step_timer == 0.0f)
        {
            glm::vec3 start = m_pPlayer->position;
            
            HitResult result_up;
            glm::vec3 end_up = m_pPlayer->position + glm::vec3(0, 0, 1024);
            m_collision.trace(result_up, start, end_up, glm::vec3(0), glm::vec3(0));
            
            HitResult result_forward;
            glm::vec3 end_forward = m_pPlayer->position + glm::vec3(1024, 0, 0);
            m_collision.trace(result_forward, start, end_forward, glm::vec3(0), glm::vec3(0));
            
            HitResult result_left;
            glm::vec3 end_left = m_pPlayer->position + glm::vec3(0, 1024, 0);
            m_collision.trace(result_left, start, end_left, glm::vec3(0), glm::vec3(0));
            
            HitResult result_back;
            glm::vec3 end_back = m_pPlayer->position + glm::vec3(-1024, 0, 0);
            m_collision.trace(result_back, start, end_back, glm::vec3(0), glm::vec3(0));
            
            HitResult result_right;
            glm::vec3 end_right = m_pPlayer->position + glm::vec3(0, -1024, 0);
            m_collision.trace(result_right, start, end_right, glm::vec3(0), glm::vec3(0));
            
            float room_depth = (result_forward.fraction + result_back.fraction) * 1024.0;
            float room_width = (result_left.fraction + result_right.fraction) * 1024.0;
            float room_height = result_up.fraction * 1024.0;
            
//            float min_distance = fmin(result_forward.fraction, result_back.fraction);
//            min_distance = fmin(min_distance, result_left.fraction);
//            min_distance = fmin(min_distance, result_right.fraction);
//            min_distance *= 1024;
            
//            printf("DEPTH: %.0fm, WIDTH: %.0fm, HEIGHT: %.0fm\n", room_depth, room_width, room_height);
            
            // Вычисление максимального расстояния
            float max_distance = sqrtf(room_width * room_width +
                                       room_height * room_height +
                                       room_depth * room_depth);
            
//            max_distance = fmax(fmax(room_width, room_height), room_depth);
            
            float decay = max_distance / 2048;
            verblib_set_room_size(&g_reverbNode.reverb, decay);
            verblib_set_damping(&g_reverbNode.reverb, 0.6);
            
            int step_id = rand() % 4;
            
            if (step_id == prev_step_id)
            {
                step_id = (step_id + 1) % 4;
            }
            
            prev_step_id = step_id;
            
            ma_sound& sound = g_sound_steps[step_id];
            
            ma_sound_set_pan(&sound, is_right_step ? 0.1 : -0.1);
            is_right_step = !is_right_step;
            
            ma_sound_start(&sound);
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
        
        cube.draw(*m_pCamera);
    }
    
    DebugRenderer::getInstance().draw(*m_pCamera);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    studio->drawViewModels(m_pCamera, m_pLightGrid.get());
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
