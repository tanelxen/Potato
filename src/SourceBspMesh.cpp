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
#include "../vendor/stb_image_write.h"

#include <unordered_map>

static SourceBSPAsset* g_bsp = nullptr;
static Shader shader;

unsigned int loadTexture(std::string filename);

#define BLOCK_SIZE 1024
GLuint allocated[BLOCK_SIZE];
float lightmap_buffer[BLOCK_SIZE*BLOCK_SIZE*3];
GLuint current_lightmap_page = 0;
GLuint g_lightmaps[8];

void LM_InitBlock();
bool LM_AllocBlock(int w, int h, int *x, int *y);
void LM_UploadBlock();

void CreateLightmapTex(ColorRGBExp32 *samples, float *dest, dface_t *face);

void SourceBspMesh::initFromBsp(SourceBSPAsset *bsp)
{
    g_bsp = bsp;
    
    faces.reserve(bsp->m_faces.size());
    
    LM_InitBlock();
    
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
        
        int textureWidth = bsp->m_materials[texinfo.texdata].width;
        int textureHeight = bsp->m_materials[texinfo.texdata].height;
        
        int lightmapWidth = bspFace.LightmapTextureSizeInLuxels[0] + 1;
        int lightmapHeight = bspFace.LightmapTextureSizeInLuxels[1] + 1;
        int lightmapX = 0;
        int lightmapY = 0;
        
        if (bspFace.lightofs != -1)
        {
            if(!LM_AllocBlock(lightmapWidth, lightmapHeight, &lightmapX, &lightmapY))
            {
                LM_UploadBlock();
                LM_InitBlock();
                
                LM_AllocBlock(lightmapWidth, lightmapHeight, &lightmapX, &lightmapY);
            }
            
            ColorRGBExp32 *samples = bsp->m_lightmap.data();
            float *dest = lightmap_buffer + (lightmapY * BLOCK_SIZE + lightmapX) * 3;
            CreateLightmapTex(samples, dest, &bspFace);
            
            face.lightmap = current_lightmap_page;
        }
        
        auto& plane = bsp->m_planes[bspFace.planenum];
        
        std::vector<mvert_t> faceVerts;
        
        for (int i = 0; i < bspFace.numedges; ++i)
        {
            int surfEdge = bsp->m_surfedges[bspFace.firstedge + i];
            int edgeIndex = abs(surfEdge);
            
            auto& edge = bsp->m_edges[edgeIndex];
            
            auto& pos = bsp->m_verts[edge.v[(surfEdge < 0) ? 1 : 0]];
            
            glm::vec2 texUV;
            texUV.x = (glm::dot(pos, texinfo.textureVecS) + texinfo.textureOffsetS) / textureWidth;
            texUV.y = (glm::dot(pos, texinfo.textureVecT) + texinfo.textureOffsetT) / textureHeight;
            
            glm::vec2 lightmapUV;
            
            if (bspFace.lightofs != -1)
            {
                float offsetS = texinfo.lightmapSOffset - bspFace.LightmapTextureMinsInLuxels[0] + lightmapX;
                float offsetT = texinfo.lightmapTOffset - bspFace.LightmapTextureMinsInLuxels[1] + lightmapY;
                
                lightmapUV.x = (glm::dot(pos, texinfo.lightmapSVec) + offsetS + 0.5f) / BLOCK_SIZE;
                lightmapUV.y = (glm::dot(pos, texinfo.lightmapTVec) + offsetT + 0.5f) / BLOCK_SIZE;
            }
            
            mvert_t vert;
            vert.pos = pos;
            vert.clr = color;
            vert.nrm = plane.normal;
            vert.uv1 = texUV;
            vert.uv2 = lightmapUV;
            
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
    
    LM_UploadBlock();
    
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
    
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, missing_id);
    
    for (auto& face : faces)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures[face.material]);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_lightmaps[face.lightmap]);
        
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
    
    glEnableVertexAttribArray(VERT_LIGHTMAP_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_LIGHTMAP_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(mvert_t), (void*)offsetof(mvert_t, uv2));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.init("assets/shaders/vbsp.glsl");
    shader.bind();
    glUniform1i(glGetUniformLocation(shader.program, "s_bspTexture"), 0);
    glUniform1i(glGetUniformLocation(shader.program, "s_bspLightmap"), 1);
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



void CreateLightmapTex(ColorRGBExp32 *samples, float *dest, dface_t *face)
{
    ColorRGBExp32 *sample;
    int lightmapW = face->LightmapTextureSizeInLuxels[0] + 1;
    int lightmapH = face->LightmapTextureSizeInLuxels[1] + 1;
    
    if(lightmapW * lightmapH == 0 || lightmapW * lightmapH > 20000)
    {
//        LOG("face %d: lightmap size %dx%d\n", i, lightmapW, lightmapH);
        return;
    }

    float p = 1.0f/2.2f;
    sample = samples+(face->lightofs/4);
    
    for(int y = 0; y < lightmapH; y++)
    {
        for(int x = 0; x < lightmapW; x++)
        {
            float m = glm::pow(2.0f, sample->exponent) / 255.0;
            
            dest[0] = sample->r * m;
            dest[1] = sample->g * m;
            dest[2] = sample->b * m;
            
            dest += 3;
            sample++;
        }
        
        dest += (BLOCK_SIZE - lightmapW) * 3;
    }
}

void LM_InitBlock()
{
    memset(allocated, 0, sizeof(allocated));
    memset(lightmap_buffer, 0, BLOCK_SIZE*BLOCK_SIZE*3);
}

bool LM_AllocBlock(int w, int h, int *x, int *y)
{
    int    i, j;
    GLuint    best, best2;
    
    best = BLOCK_SIZE;
    
    for( i = 0; i < BLOCK_SIZE - w; i++ )
    {
        best2 = 0;
        
        for( j = 0; j < w; j++ )
        {
            if( allocated[i+j] >= best )
                break;
            if( allocated[i+j] > best2 )
                best2 = allocated[i+j];
        }
        
        if( j == w )
        {
            // this is a valid spot
            *x = i;
            *y = best = best2;
        }
    }
    
    if( best + h > BLOCK_SIZE )
        return false;
    
    for( i = 0; i < w; i++ )
        allocated[*x + i] = best + h;
    
    return true;
}

//#include <format>

void LM_UploadBlock()
{
    printf("Upload lightmap block %d\n", current_lightmap_page);
    
//    std::string filename = std::format("lightmap_{}.png", current_lightmap_texture);
//    stbi_write_png(filename.c_str(), BLOCK_SIZE, BLOCK_SIZE, 3, lightmap_buffer, BLOCK_SIZE * 3);
    
    GLuint id;
    glGenTextures(1, &id);
    
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BLOCK_SIZE, BLOCK_SIZE, 0, GL_RGB, GL_FLOAT, lightmap_buffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    g_lightmaps[current_lightmap_page] = id;
    current_lightmap_page++;
}
