#version 330 core

in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uColor;
uniform vec3 uLightDir;

out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 light_dir = normalize(uLightDir);

    float diffuse = max(dot(normal, light_dir), 0.0);
    float ambient = 0.28;

    vec3 color = uColor * (ambient + diffuse * 0.72);

    FragColor = vec4(color, 1.0);
}
