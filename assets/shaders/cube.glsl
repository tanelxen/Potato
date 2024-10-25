#shader vertex
#version 410 core

uniform mat4 MVP;
layout (location = 0) in vec4 position;

void main()
{
    gl_Position = MVP * position;
}

#shader fragment
#version 410 core

uniform vec4 color;

//final color
out vec4 FragColor;

void main()
{
    FragColor = color;
}
