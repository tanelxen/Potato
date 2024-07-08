#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 in_texCoord;
layout (location = 2) in vec2 in_lmapCoord;

out vec2 g_TexCoord;
out vec2 g_LmapCoord;

void main()
{
    gl_Position = MVP * position;

    g_TexCoord = vec2(in_texCoord.x, in_texCoord.y);
    g_LmapCoord = vec2(in_lmapCoord.x, in_lmapCoord.y);
}

#shader fragment
#version 410 core

in vec2 g_TexCoord;
in vec2 g_LmapCoord;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;

//final color
out vec4 FragColor;

void main()
{
    vec4 o_texture  = texture(s_bspTexture,  g_TexCoord);
    vec4 o_lightmap = texture(s_bspLightmap, g_LmapCoord);

    FragColor = o_texture;// vec4(0.5, 0, 0.5, 1);
}
