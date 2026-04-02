#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;
uniform vec3 uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform bool uHasTexture;

uniform sampler2D uBaseColorTexture;

out vec4 FragColor;

void main()
{
    vec3 base_color = uHasTexture ? texture(uBaseColorTexture, TexCoord).rgb : uBaseColor;

    // 如果没有纹理，输出纯色以便调试
    if (!uHasTexture) {
        FragColor = vec4(base_color, 1.0);
        return;
    }

    // Ambient
    float ambient_strength = 0.15;
    vec3 ambient = ambient_strength * uLightColor * base_color;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * uLightColor * base_color;

    // Specular (simple Blinn-Phong)
    float specular_strength = 0.5 * (1.0 - uRoughness);
    vec3 view_dir = normalize(uViewPos - FragPos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), 32.0);
    vec3 specular = specular_strength * spec * uLightColor;

    vec3 result = ambient + diffuse + specular;

    // Gamma correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
