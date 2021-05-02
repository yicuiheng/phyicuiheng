#version 430 core

in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;
in vec2 Coord;

out vec3 color;

uniform vec3 LightPosition_worldspace;

vec3 calcColor(vec2 coord,float n) {
    vec3 color1 = vec3(1.0, 0.2, 0.2);
    vec3 color2 = vec3(0.9, 0.8, 0.8);

    coord = coord * n;
    float a = mod(floor(coord.x) + floor(coord.y), 2.0);
    return mix(color1, color2, a);
}

void main()
{
    vec3 LightColor = vec3(1,1,1);
    float LightPower = 50.0f;

    // Material properties
    vec3 MaterialDiffuseColor = calcColor(Coord, 15.0);
    vec3 MaterialAmbientColor = vec3(0.1, 0.1, 0.1) * MaterialDiffuseColor;

    float distance = length(LightPosition_worldspace - Position_worldspace);

    vec3 n = normalize(Normal_cameraspace);
    vec3 l = normalize(LightDirection_cameraspace);
    float cosTheta = clamp(dot(n, l), 0, 1);

    color =
        MaterialAmbientColor +
        MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance);
}