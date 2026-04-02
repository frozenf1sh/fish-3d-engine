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

uniform vec3 uLightDir;

uniform sampler2D uBaseColorTexture;

out vec4 FragColor;

void main()
{
    // 先尝试显示纹理，如果没有就用基色
    vec3 base_color = uHasTexture ? texture(uBaseColorTexture, TexCoord).rgb : uBaseColor;

    // 简单的漫反射光照，让我们看到明暗变化
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(uLightDir);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * uLightColor * base_color;

    // 加一点环境光
    vec3 ambient = 0.2 * base_color;

    vec3 result = ambient + diffuse;

    // Gamma 校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
