
#include "Quake3Bsp.h"

#include <algorithm> // std::sort
#include <cstring>   // GCC7 fix
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"


CQuake3BSP::CQuake3BSP()
{
    m_numOfVerts = 0;
    m_numOfFaces = 0;
    m_numOfIndices = 0;
    m_numOfTextures = 0;
    m_numOfLightmaps = 0;
    numVisibleFaces = 0;
    skipindices = 0;

    m_pVerts = NULL;
    m_pFaces = NULL;
    m_pIndices = NULL;
}

bool CQuake3BSP::LoadBSP(const char* filename)
{

    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter");
        return 0;
    }

    FILE* fp = NULL;
    if ((fp = fopen(filename, "rb")) == NULL) {
        printf("ERROR:: cannot open BSP file: %s\n", filename);
        return 0;
    }

    // Initialize the header and lump structures
    tBSPHeader header = {0};
    tBSPLump   lumps[kMaxLumps] = {0};

    // Read in the header and lump data
    fread(&header, 1, sizeof(tBSPHeader), fp);
    fread(&lumps, kMaxLumps, sizeof(tBSPLump), fp);

    // Allocate the vertex memory
    m_numOfVerts = lumps[kVertices].length / sizeof(tBSPVertex);
    m_pVerts = new tBSPVertex[m_numOfVerts];

    // Allocate the face memory
    m_numOfFaces = lumps[kFaces].length / sizeof(tBSPFace);
    m_pFaces = new tBSPFace[m_numOfFaces];

    // Allocate the index memory
    m_numOfIndices = lumps[kIndices].length / sizeof(int);
    m_pIndices = new int[m_numOfIndices];

    // Allocate memory to read in the texture information.
    // We create a local pointer of tBSPTextures because we don't need
    // that information once we create texture maps from it.
    m_numOfTextures = lumps[kTextures].length / sizeof(tBSPTexture);

    // Seek to the position in the file that stores the vertex information
    fseek(fp, lumps[kVertices].offset, SEEK_SET);

    // Since Quake has the Z-axis pointing up, we want to convert the data so
    // that Y-axis is pointing up (like normal!) :)
    // Go through all of the vertices that need to be read
    for (int i = 0; i < m_numOfVerts; i++)
    {
        // Read in the current vertex
        fread(&m_pVerts[i], 1, sizeof(tBSPVertex), fp);
        // Swap the y and z values, and negate the new z so Y is up.
        float temp = m_pVerts[i].vPosition.y;
        m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
        m_pVerts[i].vPosition.z = -temp;
    }

    tBSPTexture* pTextures = new tBSPTexture[m_numOfTextures];

    fseek(fp, lumps[kIndices].offset, SEEK_SET);                // Seek the index information
    fread(m_pIndices, m_numOfIndices, sizeof(int), fp);         // index information
    fseek(fp, lumps[kFaces].offset, SEEK_SET);                  // Seek the face information
    fread(m_pFaces, m_numOfFaces, sizeof(tBSPFace), fp);        // face information
    fseek(fp, lumps[kTextures].offset, SEEK_SET);               // Seek the texture information
    fread(pTextures, m_numOfTextures, sizeof(tBSPTexture), fp); // texture information

    // Create a texture from the image
    for (int i = 0; i < m_numOfTextures; i++) {
        // Find the extension if any and append it to the file name
        strcpy(tname[i], pTextures[i].strName);
        strcat(tname[i], ".jpg");
        printf("loading: %s \n", tname[i]);
    }

    m_numOfLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
    tBSPLightmap* pLightmaps = new tBSPLightmap[m_numOfLightmaps];
    // Seek to the position in the file that stores the lightmap information
    fseek(fp, lumps[kLightmaps].offset, SEEK_SET);

    // Go through all of the lightmaps and read them in
    for (int i = 0; i < m_numOfLightmaps; i++) {
        // Read in the RGB data for each lightmap
        fread(&pLightmaps[i], 1, sizeof(tBSPLightmap), fp);
        Rbuffers.lightMaps.push_back(pLightmaps[i]);
    }
    delete[] pTextures;
    delete[] pLightmaps;

    fclose(fp);
    return (fp);
}

// int skipindices = 0;
void CQuake3BSP::BuildVBO()
{
    for (int index = 0; index < m_numOfFaces; index++)
    {
        tBSPFace* pFace = &m_pFaces[index];

        if (pFace->type == FACE_POLYGON)
            skipindices += pFace->numOfIndices;

        CreateVBO(index);
        CreateIndices(index);
        CreateRenderBuffers(index);
        // BSPDebug(index);
        numVisibleFaces++;
    }

//    for (auto& f : Rbuffers.v_faceVBOs)
//    {
//        BSPDebug(f.first);
//    }
}

void CQuake3BSP::CreateVBO(int index)
{
    tBSPFace* pFace = &m_pFaces[index];

    for (int v = 0; v < pFace->numOfVerts; v++)
    {
        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.x);
        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.y);
        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.z);

        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vTextureCoord.x);
        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vTextureCoord.y);

        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vLightmapCoord.x);
        Rbuffers.faceVerts[index].push_back(m_pVerts[pFace->startVertIndex + v].vLightmapCoord.y);
    }
}

void CQuake3BSP::CreateIndices(int index)
{
    tBSPFace* pFace = &m_pFaces[index];

    for (int j = 0; j < pFace->numOfIndices; j++)
    {
        Rbuffers.faceIndices[index].push_back(m_pIndices[j + pFace->startIndex]);
    }
}

void CQuake3BSP::CreateRenderBuffers(int index)
{
    glGenVertexArrays(1, &(FB_array.FB_Idx[index].VAO));
    glBindVertexArray(FB_array.FB_Idx[index].VAO);

    glGenBuffers(1, &(FB_array.FB_Idx[index].VBO));
    glBindBuffer(GL_ARRAY_BUFFER, FB_array.FB_Idx[index].VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * Rbuffers.faceVerts[index].size(), &Rbuffers.faceVerts[index].front(), GL_STATIC_DRAW);

    glGenBuffers(1, &(FB_array.FB_Idx[index].EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FB_array.FB_Idx[index].EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * Rbuffers.faceIndices[index].size(), &Rbuffers.faceIndices[index].front(), GL_STATIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);

    // texture uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    // lightmap uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
}

void CQuake3BSP::RenderSingleFace(int index)
{
    tBSPFace* pFace = &m_pFaces[index];
    GLuint    TexID = Rbuffers.tx_ID[index];
    // GLuint LmID = Rbuffers.lm_ID[index];

    glBindVertexArray(FB_array.FB_Idx[index].VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FB_array.FB_Idx[index].EBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TexID);

//    glActiveTexture(GL_TEXTURE1);
//    if (pFace->lightmapID >= 0)
//        glBindTexture(GL_TEXTURE_2D, m_lightmap_gen_IDs[pFace->lightmapID]);
//    else
//        glBindTexture(GL_TEXTURE_2D, missing_LM_id);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    glDrawElements(GL_TRIANGLES, Rbuffers.faceIndices[index].size(), GL_UNSIGNED_INT, 0);
}

void CQuake3BSP::GenerateTexture()
{
    // GLfloat aniso = 8.0;
    GLuint textureID;
    int    width, height;
    int    num_channels = 3;

    GLuint missing_id = 1;
    glGenTextures(1, &missing_id); // generate missing texture

    unsigned char* image =
        stbi_load("assets/textures/_engine/missing.png", &width, &height, &num_channels, 3);

    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);
    glBindTexture(GL_TEXTURE_2D, missing_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);

    for (int i = 0; i < m_numOfFaces; i++) // copy to face vector
    {
        tBSPFace* Face = &m_pFaces[i];
        Rbuffers.tx_ID[i] = Face->textureID + 2;
    }

    std::vector<GLuint> missing_tex_id;

    for (int i = 0; i < m_numOfTextures; i++)
    {
        std::string path = "assets/";
        path.append(tname[i]);

        unsigned char* image = stbi_load(path.c_str(), &width, &height, &num_channels, 3);

        // filter and arrange
        if (image)
        {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
            // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Set our texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Set texture filtering

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(image);

            std::cout << "glGenTexture id: " << textureID << std::endl;
        }
        else
        {
            stbi_image_free(image);
            glGenTextures(1, &textureID);
            std::cout << "glGenTexture: " << textureID << std::endl;
            missing_tex_id.push_back(textureID);      // push missing ids
            glBindTexture(GL_TEXTURE_2D, missing_id); // bind id: 1
        }
    }

    // if missing texture - replace it with id: 1
    for (GLuint i = 0; i < Rbuffers.tx_ID.size(); i++)
    {
        for (auto val: missing_tex_id)
        {
            if (Rbuffers.tx_ID[i] == val) {
                Rbuffers.tx_ID[i] = missing_id;
            }
        }
    }

} // end

void CQuake3BSP::GenerateLightmap()
{
    // GLfloat aniso = 8.0f;

    std::ofstream logfile;
    logfile.open("log.txt");
    logfile << "LOG::\n\n";

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

    logfile << "lm START ID: " << missing_LM_id << "\n";
    logfile << "--------------\n";

    // generate lightmaps
    m_lightmap_gen_IDs = new GLuint[m_numOfLightmaps];
    glGenTextures(Rbuffers.lightMaps.size(), m_lightmap_gen_IDs);

    for (GLuint i = 0; i < Rbuffers.lightMaps.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, m_lightmap_gen_IDs[i]);
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &Rbuffers.lightMaps[i].imageBits);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    logfile << "m_numOfLightmaps " << m_numOfLightmaps;

    logfile.close();
}

void CQuake3BSP::renderFaces()
{
    for (auto& f : Rbuffers.faceVerts)
    {
        RenderSingleFace(f.first);
    }
}
