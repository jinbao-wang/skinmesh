#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 Color;

uniform sampler2D Texture0;
uniform vec3 LightPos=vec3(0.0f, 10.0f, 100.0f);
uniform vec3 ObjectColor=vec3(1.0f, 1.0f, 1.0f);

void main() {

    vec3 LightColor=vec3(texture(Texture0, TexCoords));

    //ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * LightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * LightColor;

    vec3 result = (ambient + diffuse) * ObjectColor;
    Color = vec4(result, 1.0f);

    //Color = texture(Texture0, TexCoords);
}
