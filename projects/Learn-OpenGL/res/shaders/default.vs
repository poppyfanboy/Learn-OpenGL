#version 430

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_textureCoordinates;

out vec3 Normal;
out vec3 FragmentPosition;
out vec2 TextureCoordinates;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
    Normal = mat3(transpose(inverse(u_view * u_model))) * a_normal;
    FragmentPosition = vec3(u_view * u_model * vec4(a_position, 1.0));
    TextureCoordinates = a_textureCoordinates;
}
