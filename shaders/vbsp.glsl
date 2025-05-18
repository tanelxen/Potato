#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 in_texCoord;
layout (location = 4) in vec2 in_lmapCoord;

out vec2 g_TexCoord;
out vec2 g_LmapCoord;
out vec3 g_color;
out float shade;

float shadeForNormal(vec4 normal);

void main()
{
    gl_Position = MVP * position;
    g_TexCoord = in_texCoord;
    g_LmapCoord = in_lmapCoord;
    shade = shadeForNormal(normal);
    g_color = color;
}

const float lightaxis[3] = float[](0.6f, 0.8f, 1.0f);

float shadeForNormal(vec4 normal)
{
    int i;
    float f;
    
    // axial plane
    for ( i = 0; i < 3; i++ ) {
        if (abs(normal[i]) > 0.9) {
            f = lightaxis[i];
            return f;
        }
    }
    
    // between two axial planes
    for (i = 0; i < 3; i++) {
        if (abs(normal[i]) < 0.1) {
            f = ( lightaxis[( i + 1 ) % 3] + lightaxis[( i + 2 ) % 3] ) / 2;
            return f;
        }
    }
    
    // other
    f = ( lightaxis[0] + lightaxis[1] + lightaxis[2] ) / 3;
    return f;
}

#shader fragment
#version 410 core

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_color;
in float shade;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;

//final color
out vec4 FragColor;

vec3 adjustExposure(vec3 color, float value) {
    return (1.0 + value) * color;
}

vec4 fromLinear(vec4 linearRGB)
{
    bvec4 cutoff = lessThan(linearRGB, vec4(0.0031308));
    vec4 higher = vec4(1.055)*pow(linearRGB, vec4(1.0/2.4)) - vec4(0.055);
    vec4 lower = linearRGB * vec4(12.92);

    return mix(higher, lower, cutoff);
}

void main()
{
    vec4 o_texture  = texture(s_bspTexture, g_TexCoord);
    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord);
    
    FragColor = o_texture * o_lightmap;// * vec4(shade, shade, shade, 1.0);
    
//    FragColor.rgb = adjustExposure(FragColor.rgb, 3);
}
