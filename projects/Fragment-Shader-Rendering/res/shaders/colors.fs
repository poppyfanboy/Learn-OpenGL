#version 430

out vec4 FragColor;

#define PI 3.14159265359

uniform float u_time;
uniform float u_duration;
uniform vec2 u_resolution;

mat2 rotate2d(float angle)
{
    return mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
}

float circle(vec2 uv, vec2 position, float radius, float blurSize)
{
    float distance = length(uv - position);
    float color = smoothstep(radius, radius - blurSize, distance);

    return color;
}

float face(vec2 uv, vec2 position, float radius, float blur)
{
    uv -= position;
    uv /= radius;

    float face = circle(uv, vec2(0.0, 0.0), 1.0, blur);
    face -= circle(uv, vec2(-0.33, 0.1), 0.17, blur);
    face -= circle(uv, vec2(0.33, 0.1), 0.17, blur);

    float mouth = circle(uv, vec2(0.0, -0.5), 0.33, blur);
    mouth -= circle(uv, vec2(0.0, -0.7), 0.4, blur);
    mouth = clamp(mouth, 0.0, 1.0);
    face -= mouth;

    return face;
}

void main()
{
    float timeLooped = u_time / u_duration;
    float cellSize = 0.05 * (sin(timeLooped * 2.0 * PI) + 1.5) * 2.0;

    vec2 uv = vec2(gl_FragCoord) / u_resolution.xy;
    vec2 scaledCoords = uv - 0.5;
    scaledCoords = vec2(scaledCoords.x * u_resolution.x / u_resolution.y, scaledCoords.y);

    vec2 rotatedCoords =
        rotate2d(mod(-timeLooped * 2.0 * PI, 2.0 * PI)) * (scaledCoords - vec2(0.5));
    rotatedCoords = mod(rotatedCoords, cellSize);

    vec3 color = 0.5 + 0.5 * sin(timeLooped * 2.0 * PI + uv.yxx + vec3(1, 0, 2));
    float face = face(rotatedCoords, vec2(cellSize / 2.0), cellSize / 2.5, 0.01);

    FragColor = vec4(vec3(face) * color, 1.0);
}
