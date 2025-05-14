//
//  SourceBspMesh.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 11.05.25.
//

#include "SourceBspMesh.h"
#include "SourceBSPAsset.h"

#include "Camera.h"
#include "Shader.h"

#include <glad/glad.h>
#include "../vendor/stb_image.h"

#include <unordered_map>

static SourceBSPAsset* g_bsp = nullptr;
static Shader shader;

unsigned int loadTexture(std::string filename);

void SourceBspMesh::initFromBsp(SourceBSPAsset *bsp)
{
    g_bsp = bsp;
    
    faces.reserve(bsp->m_faces.size());
    
    for (auto& bspFace : bsp->m_faces)
    {
        if (bspFace.dispinfo != -1) continue;
        
        auto& texinfo = bsp->m_texinfos[bspFace.texinfo];
        
        auto& texture_name = bsp->m_materials[texinfo.texdata].name;
        if (texture_name == "tools/toolstrigger") continue;
        
        mface_t& face = faces.emplace_back();
        
        face.firstVert = indices.size();
        face.numVerts = 0;
        
        face.material = texinfo.texdata;
        
        auto& reflect = bsp->m_materials[texinfo.texdata].reflecivity;
        glm::vec3 color = glm::vec3(sqrt(reflect.x), sqrt(reflect.y), sqrt(reflect.z));
        
        auto s = texinfo.textureVecs[0];
        auto t = texinfo.textureVecs[1];
        
        int width = bsp->m_materials[texinfo.texdata].width;
        int height = bsp->m_materials[texinfo.texdata].height;
        
        auto& plane = bsp->m_planes[bspFace.planenum];
        
        std::vector<mvert_t> faceVerts;
        
        for (int i = 0; i < bspFace.numedges; ++i)
        {
            int surfEdge = bsp->m_surfedges[bspFace.firstedge + i];
            int edgeIndex = abs(surfEdge);
            
            auto& edge = bsp->m_edges[edgeIndex];
            
            auto& v1 = bsp->m_verts[edge.v[(surfEdge < 0) ? 1 : 0]];
            
            float u = (v1.x * s[0] + v1.y * s[1] + v1.z * s[2] + s[3]) / width;
            float v = (v1.x * t[0] + v1.y * t[1] + v1.z * t[2] + t[3]) / height;
            
            mvert_t vert;
            vert.pos = v1;
            vert.clr = color;
            vert.nrm = plane.normal;
            vert.uv1 = glm::vec2(u, v);
            
            faceVerts.push_back(vert);
        }
        
        uint32_t baseIndex = verts.size();
        verts.insert(verts.end(), faceVerts.begin(), faceVerts.end());
        
        for (int i = 2; i < faceVerts.size(); ++i)
        {
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + i - 1);
            indices.push_back(baseIndex + i);
            
            face.numVerts += 3;
        }
    }
    
    initBuffers();
    generateTextures();
}

void SourceBspMesh::renderFaces(glm::mat4x4 &mvp)
{
    shader.bind();
    shader.setUniform("MVP", mvp);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, missing_id);
    
    for (auto& face : faces)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[face.material]);
        glDrawElements(GL_TRIANGLES, face.numVerts, GL_UNSIGNED_INT, (void *)(face.firstVert * sizeof(uint32_t)));
    }
}

#define VERT_POSITION_LOC 0
#define VERT_NORMAL_LOC 1
#define VERT_COLOR_LOC 2
#define VERT_DIFFUSE_TEX_COORD_LOC 3
#define VERT_LIGHTMAP_TEX_COORD_LOC 4

void SourceBspMesh::initBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mvert_t) * verts.size(), verts.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(VERT_POSITION_LOC);
    glVertexAttribPointer(VERT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(mvert_t), (void*)offsetof(mvert_t, pos));
    
    glEnableVertexAttribArray(VERT_NORMAL_LOC);
    glVertexAttribPointer(VERT_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(mvert_t), (void*)offsetof(mvert_t, nrm));
    
    glEnableVertexAttribArray(VERT_COLOR_LOC);
    glVertexAttribPointer(VERT_COLOR_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(mvert_t), (void*)offsetof(mvert_t, clr));
    
    glEnableVertexAttribArray(VERT_DIFFUSE_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_DIFFUSE_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(mvert_t), (void*)offsetof(mvert_t, uv1));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.init("assets/shaders/vbsp.glsl");
    shader.bind();
    glUniform1i(glGetUniformLocation(shader.program, "s_bspTexture"), 0);
    shader.unbind();
}

void SourceBspMesh::generateTextures()
{
    int width, height;
    int num_channels = 3;
    
//    GLuint missing_id;
    glGenTextures(1, &missing_id); // generate missing texture
    
    unsigned char* image = stbi_load("assets/textures/_engine/missing.png", &width, &height, &num_channels, 3);
    
    glBindTexture(GL_TEXTURE_2D, missing_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);
    
    std::unordered_map<std::string, GLuint> cache;
    
    for (int i = 0; i < g_bsp->m_materials.size(); i++)
    {
        auto& name = g_bsp->m_materials[i].name;
        
        if (cache.contains(name))
        {
            m_textures[i] = cache.at(name);
            continue;
        }
        
        std::string path = "assets/hl2/materials/";
        path.append(g_bsp->m_materials[i].name);
        path.append(".vtf");
        
        GLuint id = loadTexture(path);
        
        if (id == 0) id = missing_id;
        
        cache[name] = id;
        m_textures[i] = id;
    }
}
