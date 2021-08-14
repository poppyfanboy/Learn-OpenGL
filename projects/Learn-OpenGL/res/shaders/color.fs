#version 430

out vec4 FragmentColor;

uniform vec3 u_color;

void main()
{
    FragmentColor = vec4(u_color, 1.0);
}
