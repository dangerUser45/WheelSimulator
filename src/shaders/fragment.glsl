#version 330 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uViewPos;
uniform float uSpecularStrength;
uniform float uShininess;

out vec4 FragColor;

vec3 AmbientColor(vec3 color)
{
    return color * 0.34;
}

vec3 DiffuseColor(vec3 color, vec3 normal, vec3 light_dir)
{
    float diffuse = max(dot(normal, light_dir), 0.0);
    return color * diffuse * 0.76;
}

vec3 SpecularColor(vec3 normal, vec3 light_dir, vec3 view_dir)
{
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular = pow(max(dot(view_dir, reflect_dir), 0.0), uShininess);
    return vec3(1.0) * specular * uSpecularStrength;
}

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 light_dir = normalize(uLightDir);
    vec3 view_dir = normalize(uViewPos - vWorldPosition);

    vec3 color = AmbientColor(uColor);
    color += DiffuseColor(uColor, normal, light_dir);
    color += SpecularColor(normal, light_dir, view_dir);

    FragColor = vec4(color, 1.0);
}
