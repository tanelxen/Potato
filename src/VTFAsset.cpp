//
//  VTFAsset.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 12.05.25.
//

#include "VTFAsset.h"
#include <glad/glad.h>

enum ImageFormat
{
    IMAGE_FORMAT_UNKNOWN  = -1,
    IMAGE_FORMAT_RGBA8888 = 0,
    IMAGE_FORMAT_ABGR8888,
    IMAGE_FORMAT_RGB888,
    IMAGE_FORMAT_BGR888,
    IMAGE_FORMAT_RGB565,
    IMAGE_FORMAT_I8,
    IMAGE_FORMAT_IA88,
    IMAGE_FORMAT_P8,
    IMAGE_FORMAT_A8,
    IMAGE_FORMAT_RGB888_BLUESCREEN,
    IMAGE_FORMAT_BGR888_BLUESCREEN,
    IMAGE_FORMAT_ARGB8888,
    IMAGE_FORMAT_BGRA8888,
    IMAGE_FORMAT_DXT1,
    IMAGE_FORMAT_DXT3,
    IMAGE_FORMAT_DXT5,
    IMAGE_FORMAT_BGRX8888,
    IMAGE_FORMAT_BGR565,
    IMAGE_FORMAT_BGRX5551,
    IMAGE_FORMAT_BGRA4444,
    IMAGE_FORMAT_DXT1_ONEBITALPHA,
    IMAGE_FORMAT_BGRA5551,
    IMAGE_FORMAT_UV88,
    IMAGE_FORMAT_UVWQ8888,
    IMAGE_FORMAT_RGBA16161616F,
    IMAGE_FORMAT_RGBA16161616,
    IMAGE_FORMAT_UVLX8888,
    IMAGE_FORMAT_R32F,            // Single-channel 32-bit floating point
    IMAGE_FORMAT_RGB323232F,
    IMAGE_FORMAT_RGBA32323232F,
    
    // Depth-stencil texture formats for shadow depth mapping
    IMAGE_FORMAT_NV_DST16,        //
    IMAGE_FORMAT_NV_DST24,        //
    IMAGE_FORMAT_NV_INTZ,        // Vendor-specific depth-stencil texture
    IMAGE_FORMAT_NV_RAWZ,        // formats for shadow depth mapping
    IMAGE_FORMAT_ATI_DST16,        //
    IMAGE_FORMAT_ATI_DST24,        //
    IMAGE_FORMAT_NV_NULL,        // Dummy format which takes no video memory
    
    // Compressed normal map formats
    IMAGE_FORMAT_ATI2N,            // One-surface ATI2N / DXN format
    IMAGE_FORMAT_ATI1N,            // Two-surface ATI1N format
    
    NUM_IMAGE_FORMATS
};

#pragma pack(push, 1)
struct VTFFileHeader_t
{
    char fileTypeString[4];
    int version[2];
    int headerSize;
    
    unsigned short  width;
    unsigned short  height;
    unsigned int    flags;
    unsigned short  numFrames;
    unsigned short  startFrame;
    char            pad1[4];
    float           reflectivity[3];
    char            pad2[4];
    float           bumpScale;
    ImageFormat     imageFormat;
    unsigned char   numMipLevels;
    ImageFormat     lowResImageFormat;
    unsigned char   lowResImageWidth;
    unsigned char   lowResImageHeight;
};
#pragma pack(pop)

size_t computeMipsSize(int width, int height, int mipcount, float bpp);

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

//#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
//#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
//#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
//#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

unsigned int loadVTFTexture(std::string filename)
{
    FILE* fp = fopen(filename.c_str(), "rb" );
    
    if(fp == nullptr)
    {
        printf("unable to open %s\n", filename.c_str());
        return 0;
    }
    
//    printf("Read %s\n", filename.c_str());
    
    VTFFileHeader_t header;
    fread(&header, 1, sizeof(VTFFileHeader_t), fp);
    
    int mip = 0;
    
    if (header.flags & 0x4000) mip = 0;
    if (mip >= header.numMipLevels) mip = header.numMipLevels - 1;
    
    int width = header.width >> mip;
    int height = header.height >> mip;
    
    if (width < 4) width = 4;
    if (height < 4) height = 4;
    
    
    
    size_t texSize = 0;
    size_t offset = header.headerSize;
    
    int faces = 1;
    
    if (header.flags & 0x4000)
    {
        if (header.version[1] < 5) faces = 7;
        else faces = 6;
    }
    else
    {
        offset += ((header.lowResImageWidth + 3) / 4) * ((header.lowResImageHeight + 3) / 4) * 8;
    }

    
    int ifmt;
    int fmt;
    int type = GL_UNSIGNED_BYTE;
    bool compressed = false;
    
    if (header.imageFormat == IMAGE_FORMAT_DXT1)
    {
        texSize = ((width + 3) / 4) * ((height + 3) / 4) * 8;
        offset += computeMipsSize(width, height, header.numMipLevels, 0.5f) * faces;
        ifmt = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        fmt = GL_RGB;
        compressed=true;
    }
    else if (header.imageFormat == IMAGE_FORMAT_DXT5)
    {
        texSize = ((width + 3) / 4) * ((height + 3) / 4) * 16;
        offset += computeMipsSize(width,height,header.numMipLevels, 1) * faces;
        ifmt = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        fmt = GL_RGBA;
        compressed=true;
    }
//    else if (header.imageFormat == IMAGE_FORMAT_RGBA16161616F)
//    {
//        texSize = width * height * 8;
//        offset += computeMipsSize(width,height,header.numMipLevels,8)*faces;
//        ifmt = GL_RGBA16F;
//        fmt = GL_RGBA;
//        type = GL_HALF_FLOAT;
//    }
//    else if (header.imageFormat == IMAGE_FORMAT_BGRA8888)
//    {
//        texSize = width * height * 4;
//        offset += computeMipsSize(width,height,header.numMipLevels,4)*faces;
//        ifmt = GL_RGBA;
//        fmt = GL_BGRA;
//    }
    else if (header.imageFormat == IMAGE_FORMAT_BGR888)
    {
        texSize = width * height * 3;
        offset += computeMipsSize(width,height,header.numMipLevels-mip,3)*faces;
        ifmt = GL_RGB;
        fmt = GL_BGR;
        //need resample
    }
    else
    {
        printf("Unknown format %i\n", header.imageFormat);
        fclose(fp);
        return 0;
    }
    
    GLubyte *image = new GLubyte[texSize * faces];
    
    fseek(fp, offset, SEEK_SET);
    fread(image, texSize * faces, sizeof(GLubyte), fp);
    
    GLuint id;
    glGenTextures(1, &id);
    
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
    
    if (compressed)
    {
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, texSize, image);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, ifmt, width, height, 0, fmt, type, image);
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    delete [] image;
    
    return id;
}

size_t computeMipsSize(int width, int height, int mipcount, float bpp)
{
    size_t imgSize = 0;
    
    width>>=1;
    height>>=1;
    if(bpp<3)
    {
        if(width < 4)
            width = 4;
        
        if(height < 4)
            height = 4;
    }
    for(int i=0; i<mipcount-1; i++)
    {
        imgSize+=(width*height*bpp);
        
        width>>=1;
        height>>=1;
        if(bpp<3)
        {
            if(width < 4)
                width = 4;
            
            if(height < 4)
                height = 4;
        }
    }
    
    return imgSize;
}
