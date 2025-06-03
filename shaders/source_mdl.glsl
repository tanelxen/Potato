#shader vertex
#version 410 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in uint boneIndex;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 u_AmbientCube[6];

out vec2 uv;
out vec3 ambient;

vec3 AmbientLight(in vec3 t_NormalWorld);

void main()
{
    gl_Position = uMVP * position;
    uv = texCoord;
    
    vec3 n_normal = normalize(mat3(uModel) * normal);
    ambient = AmbientLight(n_normal);
}

vec3 AmbientLight(in vec3 t_NormalWorld)
{
    vec3 t_Weight = t_NormalWorld * t_NormalWorld;
    bvec3 t_Negative = lessThan(t_NormalWorld, vec3(0.0));
    return (
            t_Weight.x * u_AmbientCube[t_Negative.x ? 1 : 0].rgb +
            t_Weight.y * u_AmbientCube[t_Negative.y ? 3 : 2].rgb +
            t_Weight.z * u_AmbientCube[t_Negative.z ? 5 : 4].rgb
            );
}

#shader fragment
#version 410 core

in vec2 uv;
in vec3 ambient;

uniform sampler2D s_texture;

out vec4 FragColor;

void main()
{
    FragColor = texture(s_texture, uv);
    FragColor.rgb *= ambient;
}
