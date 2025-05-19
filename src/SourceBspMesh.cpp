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
static Shader shader_disp;

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
        if (bspFace.dispinfo != -1)
        {
            processDisplacementFace(bspFace, bsp);
        }
        else
        {
            processRegularFace(bspFace, bsp);
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
    
    for (auto& face : faces)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures[face.material]);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_lightmaps[face.lightmap]);
        
        glDrawElements(GL_TRIANGLES, face.numVerts, GL_UNSIGNED_INT, (void *)(face.firstVert * sizeof(uint32_t)));
    }
    
    shader_disp.bind();
    shader_disp.setUniform("MVP", mvp);
    
    for (auto& face : disp_faces)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures[face.material]);
        
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, g_lightmaps[face.lightmap]);
        
        glDrawArrays(GL_TRIANGLES, face.firstVert, face.numVerts);
//        glDrawElements(GL_TRIANGLES, face.numVerts, GL_UNSIGNED_INT, (void *)(face.firstVert * sizeof(uint32_t)));
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
    
    shader_disp.init("assets/shaders/vbsp_disp.glsl");
    shader_disp.bind();
    glUniform1i(glGetUniformLocation(shader_disp.program, "s_bspTexture"), 0);
    glUniform1i(glGetUniformLocation(shader_disp.program, "s_bspLightmap"), 1);
    shader_disp.unbind();
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

void SourceBspMesh::processRegularFace(dface_t &bspFace, SourceBSPAsset *bsp)
{
    auto& texinfo = bsp->m_texinfos[bspFace.texinfo];
    
    auto& texture_name = bsp->m_materials[texinfo.texdata].name;
    if (texture_name == "tools/toolstrigger") return;
    
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

glm::vec3 findNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

void SourceBspMesh::processDisplacementFace(struct dface_t &bspFace, SourceBSPAsset *bsp)
{
    glm::vec3 low_base = bsp->m_displacements[bspFace.dispinfo].startPosition;
    
    if (bspFace.numedges != 4) {
        printf("Bad displacement!\n");
        return;
    }
    
    glm::vec3 corner_verts[4];
    int base_i = -1;
    float base_dist = INFINITY;
    
    for (int k = 0; k < 4; k++)
    {
        int surfEdge = bsp->m_surfedges[bspFace.firstedge + k];
        int edgeIndex = abs(surfEdge);
        
        auto& edge = bsp->m_edges[edgeIndex];
        
        auto& pos = bsp->m_verts[edge.v[(surfEdge < 0) ? 1 : 0]];
        
        float this_dist = std::abs(pos.x - low_base.x) + std::abs(pos.y - low_base.y) + std::abs(pos.z - low_base.z);
        
        if (this_dist < base_dist)
        {
            base_dist = this_dist;
            base_i = k;
        }
        
        corner_verts[k] = pos;
    }
    
    if (base_i == -1) {
        printf("Bad base in displacement!\n");
        return;
    }
    
    auto& texinfo = bsp->m_texinfos[bspFace.texinfo];
    int textureWidth = bsp->m_materials[texinfo.texdata].width;
    int textureHeight = bsp->m_materials[texinfo.texdata].height;
    
    auto& reflect = bsp->m_materials[texinfo.texdata].reflecivity;
    glm::vec3 color = glm::vec3(sqrt(reflect.x), sqrt(reflect.y), sqrt(reflect.z));
    
    int lightmapWidth = bspFace.LightmapTextureSizeInLuxels[0] + 1;
    int lightmapHeight = bspFace.LightmapTextureSizeInLuxels[1] + 1;
    int lightmapX = 0;
    int lightmapY = 0;
    int lightmapPage = -1;
    
//    if (bspFace.lightofs != -1)
//    {
//        if(!LM_AllocBlock(lightmapWidth, lightmapHeight, &lightmapX, &lightmapY))
//        {
//            LM_UploadBlock();
//            LM_InitBlock();
//            
//            LM_AllocBlock(lightmapWidth, lightmapHeight, &lightmapX, &lightmapY);
//        }
//        
//        ColorRGBExp32 *samples = bsp->m_lightmap.data();
//        float *dest = lightmap_buffer + (lightmapY * BLOCK_SIZE + lightmapX) * 3;
//        CreateLightmapTex(samples, dest, &bspFace);
//        
//        lightmapPage = current_lightmap_page;
//    }
    
    glm::vec3 high_base = corner_verts[ (base_i + 3) % 4 ];
    glm::vec3 high_ray = corner_verts[ (base_i + 2) % 4 ] - high_base;
    glm::vec3 low_ray = corner_verts[ (base_i + 1) % 4 ] - low_base;
    
    int verts_wide = (2 << (bsp->m_displacements[bspFace.dispinfo].power - 1)) + 1;
    
    glm::vec3 base_verts_pos[289];
    glm::vec2 base_verts_uv1[289];
//    glm::vec2 base_verts_uv2[289];
    glm::vec3 base_verts_clr[289];
//    float base_alphas[289];
    
    int base_dispvert_index = bsp->m_displacements[bspFace.dispinfo].DispVertStart;
    
    for (int y = 0; y < verts_wide; y++)
    {
        float fy = (float)y / (verts_wide - 1);
        
        glm::vec3 mid_base = low_base + low_ray * fy;
        glm::vec3 mid_ray = high_base + high_ray * fy - mid_base;
        
        for (int x = 0; x < verts_wide; x++)
        {
            float fx = (float)x / (verts_wide - 1);
            int i = x + y * verts_wide;
            
            glm::vec3 offset = bsp->m_disp_verts[base_dispvert_index + i].pos;
            float scale = bsp->m_disp_verts[base_dispvert_index + i].distance;
            float alpha = bsp->m_disp_verts[base_dispvert_index + i].alpha / 255;
            
            base_verts_pos[i] = mid_base + mid_ray * fx + offset*scale;
            
            base_verts_uv1[i].x = (glm::dot(base_verts_pos[i], texinfo.textureVecS) + texinfo.textureOffsetS) / textureWidth;
            base_verts_uv1[i].y = (glm::dot(base_verts_pos[i], texinfo.textureVecT) + texinfo.textureOffsetT) / textureHeight;
            
//            if (bspFace.lightofs != -1)
//            {
//                float offsetS = texinfo.lightmapSOffset - bspFace.LightmapTextureMinsInLuxels[0] + lightmapX;
//                float offsetT = texinfo.lightmapTOffset - bspFace.LightmapTextureMinsInLuxels[1] + lightmapY;
//                
//                base_verts_uv2[i].x = (glm::dot(base_verts_pos[i], texinfo.lightmapSVec) + offsetS + 0.5f) / BLOCK_SIZE;
//                base_verts_uv2[i].y = (glm::dot(base_verts_pos[i], texinfo.lightmapTVec) + offsetT + 0.5f) / BLOCK_SIZE;
//            }
            
            base_verts_clr[i] = glm::mix(color, {1, 0, 1}, alpha);
        }
    }
    
    std::vector<mvert_t> faceVerts;
    
    for (int y = 0; y < verts_wide - 1; y++)
    {
        for (int x = 0; x < verts_wide - 1; x++)
        {
            int i = x + y * verts_wide;
            
            mvert_t vert1, vert2, vert3, vert4;
            
            vert1.pos = base_verts_pos[i];
            vert2.pos = base_verts_pos[i + 1];
            vert3.pos = base_verts_pos[i + verts_wide];
            vert4.pos = base_verts_pos[i + verts_wide + 1];
            
            vert1.uv1 = base_verts_uv1[i];
            vert2.uv1 = base_verts_uv1[i + 1];
            vert3.uv1 = base_verts_uv1[i + verts_wide];
            vert4.uv1 = base_verts_uv1[i + verts_wide + 1];
            
            vert1.clr = base_verts_clr[i];
            vert2.clr = base_verts_clr[i + 1];
            vert3.clr = base_verts_clr[i + verts_wide];
            vert4.clr = base_verts_clr[i + verts_wide + 1];
            
            if (i % 2)
            {
                faceVerts.push_back(vert1);
                faceVerts.push_back(vert3);
                faceVerts.push_back(vert2);
                
                faceVerts.push_back(vert2);
                faceVerts.push_back(vert3);
                faceVerts.push_back(vert4);
            }
            else
            {
                faceVerts.push_back(vert1);
                faceVerts.push_back(vert3);
                faceVerts.push_back(vert4);
                
                faceVerts.push_back(vert2);
                faceVerts.push_back(vert1);
                faceVerts.push_back(vert4);
            }
        }
    }
    
    mface_t& face = disp_faces.emplace_back();
    
    face.material = texinfo.texdata;
    face.firstVert = verts.size();
    face.numVerts = faceVerts.size();
    face.lightmap = lightmapPage;
    
    verts.insert(verts.end(), faceVerts.begin(), faceVerts.end());
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

glm::vec3 findNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    glm::vec3 u = b - c;
    glm::vec3 v = a - c;
    
    glm::vec3 norm;
    norm.x = u.y*v.z - u.z*v.y;
    norm.y = u.z*v.x - u.x*v.z;
    norm.z = u.x*v.y - u.y*v.x;
    
    float len = sqrt(norm.x*norm.x + norm.y*norm.y + norm.z*norm.z);
    norm.x /= len;
    norm.y /= len;
    norm.z /= len;
    
    return norm;
}
