//
//  BspScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 27.10.24.
//

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "BspScene.h"
#include "Quake3Bsp.h"
#include "Camera.h"
#include "Shader.h"

static CQuake3BSP* g_bsp = nullptr;
static Shader shader;

void BspScene::initFromBsp(CQuake3BSP* bsp)
{
    g_bsp = bsp;
    
    GenerateTexture();
    GenerateLightmap();
    initBuffers();
}

void BspScene::GenerateTexture()
{
    // GLfloat aniso = 8.0;
    GLuint textureID;
    int    width, height;
    int    num_channels = 3;

    GLuint missing_id = 1;
    glGenTextures(1, &missing_id); // generate missing texture

    unsigned char* image = stbi_load("assets/textures/_engine/missing.png", &width, &height, &num_channels, 3);

    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
    glBindTexture(GL_TEXTURE_2D, missing_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);

    std::vector<GLuint> missing_tex_id;

    for (int i = 0; i < g_bsp->m_numTextures; i++)
    {
        std::string path = "assets/";
        path.append(g_bsp->pTextures[i].strName);
        
        unsigned char* image = nullptr;
        
        std::string jpgPath = path + ".jpg";
        image = stbi_load(jpgPath.c_str(), &width, &height, &num_channels, 3);
        
        if (image == nullptr)
        {
            std::string tgaPath = path + ".tga";
            image = stbi_load(tgaPath.c_str(), &width, &height, &num_channels, 3);
        }

        if (image)
        {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
            // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);

            m_textures[i] = textureID;

            std::cout << path << std::endl;
//            std::cout << "glGenTexture id: " << textureID << std::endl;
        }
        else
        {
            m_textures[i] = missing_id;
        }
        
        stbi_image_free(image);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BspScene::GenerateLightmap()
{
    // GLfloat aniso = 8.0f;

    // generate missing lightmap
    GLfloat white_lightmap[] =
        {1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f};

    glGenTextures(1, &missing_LM_id);
    glBindTexture(GL_TEXTURE_2D, missing_LM_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGB, GL_FLOAT, &white_lightmap);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    for (GLuint i = 0; i < g_bsp->m_numLightmaps; i++)
    {
        GLuint lmId = 0;
        glGenTextures(1, &lmId);
        glBindTexture(GL_TEXTURE_2D, lmId);
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &g_bsp->pLightmaps[i]);
        glGenerateMipmap(GL_TEXTURE_2D);

        m_lightmaps[i] = lmId;
    }
}

#define VERT_POSITION_LOC 0
#define VERT_DIFFUSE_TEX_COORD_LOC 1
#define VERT_LIGHTMAP_TEX_COORD_LOC 2
#define VERT_NORMAL_LOC 3

#define FACE_POLYGON 1

void BspScene::initBuffers()
{
    std::unordered_map<int, std::vector<int>> indicesByTexture;
    
    for (int i = 0; i < g_bsp->m_numFaces; ++i)
    {
        tBSPFace &face = g_bsp->m_pFaces[i];
        
        if (face.type != FACE_POLYGON) continue;
        
        for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
        {
            unsigned int index = g_bsp->m_pIndices[j] + face.startVertIndex;
            
            indicesByTexture[face.textureID].push_back(index);
        }
    }
    
    for (auto pair : indicesByTexture)
    {
        Surface surface = {0};
        surface.texId = m_textures[pair.first];
        
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        
        surfaces.push_back(surface);
        
        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
    }
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tBSPVertex) * g_bsp->m_numVerts, g_bsp->m_pVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERT_POSITION_LOC);
    glVertexAttribPointer(VERT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), 0);

    glEnableVertexAttribArray(VERT_DIFFUSE_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_DIFFUSE_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void *)12);

    glEnableVertexAttribArray(VERT_LIGHTMAP_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_LIGHTMAP_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void *)20);

    glEnableVertexAttribArray(VERT_NORMAL_LOC);
    glVertexAttribPointer(VERT_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void *)28);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/basic.glsl");
}

void BspScene::renderFaces(Camera* camera)
{
    shader.bind();
    glUniform1i(glGetUniformLocation(shader.program, "s_bspTexture"), 0);
    glUniform1i(glGetUniformLocation(shader.program, "s_bspLightmap"), 1);
    
    glm::mat4x4 mvp = camera->projection * camera->view;
    shader.setUniformMatrix((const float*) &mvp, "MVP");
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    for (auto surface : surfaces)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.texId);
        
        glDrawElements(GL_TRIANGLES, surface.numVerts, GL_UNSIGNED_INT, (void *)surface.bufferOffset);
    }
}
