#version 430

#define PI 3.14159265359
#define ITERATIONS_COUNT 4

out vec4 FragColor;

uniform float u_time;
uniform float u_loopDuration;
uniform vec2 u_resolution;
#define DIFFUSE_TEXTURES_COUNT 1
uniform sampler2D u_diffuseTexture[DIFFUSE_TEXTURES_COUNT];

void mirrorOverLine(inout vec2 coords, float lineDirectionAngle, vec2 lineOriginCoords)
{
    vec2 mirrorLineNormal = vec2(-sin(lineDirectionAngle), cos(lineDirectionAngle));
    float distanceToMirrorLine = dot(coords - lineOriginCoords, mirrorLineNormal);
    coords -= 2.0 * mirrorLineNormal * max(distanceToMirrorLine, 0);
}

// (The line passes through the origin.)
void mirrorOverLine(inout vec2 coords, float lineDirectionAngle)
{
    mirrorOverLine(coords, lineDirectionAngle, vec2(0.0));
}

void inverseMirrorOverLine(inout vec2 coords, float lineDirectionAngle, vec2 lineOriginCoords)
{
    vec2 mirrorLineNormal = vec2(-sin(lineDirectionAngle), cos(lineDirectionAngle));
    float distanceToMirrorLine = dot(coords - lineOriginCoords, mirrorLineNormal);
    coords += 2.0 * mirrorLineNormal * max(-distanceToMirrorLine, 0);
}

// (The line passes through the origin.)
void inverseMirrorOverLine(inout vec2 coords, float lineDirectionAngle)
{
    inverseMirrorOverLine(coords, lineDirectionAngle, vec2(0.0));
}

void mirrorVertically(inout vec2 coords)
{
    coords.x = abs(coords.x);
}

void mirrorHorizontally(inout vec2 coords)
{
    coords.y = abs(coords.y);
}

void inverseMirrorVertically(inout vec2 coords)
{
    coords.x = -abs(coords.x);
}

void inverseMirrorHorizontally(inout vec2 coords)
{
    coords.y = -abs(coords.y);
}

// (Values > 1 make the image smaller. Unit length variable is needed to later
// draw on screen with the correct sizes.)
void scale(inout vec2 coords, vec2 scaleFactor, inout vec2 unitLength)
{
    coords *= scaleFactor;
    unitLength /= scaleFactor;
}

void scale(inout vec2 coords, float scaleFactor, inout vec2 unitLength)
{
    scale(coords, vec2(scaleFactor, scaleFactor), unitLength);
}

void rotate(inout vec2 coords, float angle, vec2 origin)
{
    coords *= mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

void rotate(inout vec2 coords, float angle)
{
    rotate(coords, angle, vec2(0.0));
}

void main()
{
    float timeLooped = u_time / u_loopDuration;

    vec2 uv = vec2(gl_FragCoord) / u_resolution.xy;
    vec2 coords = uv - 0.5;
    coords = vec2(coords.x * u_resolution.x / u_resolution.y, coords.y);
    vec2 tt = coords;

    vec3 color = vec3(0.0);
    vec2 unitLength = vec2(1.0);

    mirrorOverLine(coords, sin(2.0 * timeLooped) * PI);
    mirrorOverLine(coords, sin(2.0 * timeLooped) * PI + PI / 2.0);
    mirrorOverLine(coords, sin(2.0 * timeLooped) * PI + 3.0 * PI / 4.0);
    mirrorOverLine(coords, sin(2.0 * timeLooped) * PI + PI / 4.0);

    coords.y -= 0.2;
    scale(coords, 1.5, unitLength);
    inverseMirrorVertically(coords);
    inverseMirrorOverLine(
        coords, -PI / 6.0 * (sin(4.0 * timeLooped + PI / 3.0) + 1.0), vec2(-0.5, 0.0));

    scale(coords, 3.0, unitLength);
    mirrorVertically(coords);
    coords.x -= 0.5;
    mirrorOverLine(coords, (sin(6.0 * timeLooped) + 2.0) * PI / 3.0);

    for (int i = 0; i < ITERATIONS_COUNT; i++)
    {
        scale(coords, 3.0, unitLength);
        coords.x -= 1.5;

        mirrorVertically(coords);
        coords.x -= 0.5;
        mirrorOverLine(coords, PI / 3.0);
    }

    float distanceToLine = length(coords - vec2(clamp(coords.x, -1.0, 1.0), 0.0));
    color +=
        vec3(smoothstep(8.0 / u_resolution.x, 0.0, distanceToLine * unitLength.x)) * coords.xyy;
    color.gb += 0.05 * coords;
    color.r += 0.07 * coords.x;

    FragColor = vec4(mix(color,
                         vec3(texture(u_diffuseTexture[0], color.gb + 0.1 * sin(timeLooped))),
                         1 - (sin(4 * timeLooped) + 1.0) / 4.0),
                     1.0);
}
