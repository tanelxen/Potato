#shader vertex
#version 410 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in uint boneIndex;

uniform mat4 uBoneTransforms[128];
uniform mat4 uMVP;

out vec2 uv;
out vec3 transformedNormal;
out vec4 transformedPosition;

void main()
{
    transformedPosition = uBoneTransforms[boneIndex] * position;
    transformedNormal = normalize(mat3(uBoneTransforms[boneIndex]) * normal);
    gl_Position = uMVP * transformedPosition;
    uv = texCoord;
}

#shader fragment
#version 410 core
in vec2 uv;
in vec3 transformedNormal;
in vec4 transformedPosition;

//Texture samplers
uniform sampler2D s_texture;

//final color
out vec4 FragColor;

void main()
{
    vec3 lightPos = vec3(128, 128, 128);
    vec3 lightDir = normalize(lightPos - transformedPosition.xyz);
    float nDotL = dot(transformedNormal, lightDir);
    float shade = max(nDotL, 0.0);

    float ambient = 0.2;
    shade = min(shade + ambient, 1.0);

    FragColor = texture(s_texture, uv) * shade;
}
