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
    // 先获取纹理颜色
    vec3 base_color = uHasTexture ? texture(uBaseColorTexture, TexCoord).rgb : uBaseColor;

    // 调试1：直接输出法线作为颜色
    FragColor = vec4(abs(Normal) * 0.5 + 0.5, 1.0);
    return;

    // 调试2：直接输出光源方向
    // FragColor = vec4(abs(uLightDir) * 0.5 + 0.5, 1.0);
    // return;

    // 简单漫反射光照
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(uLightDir);

    // 调试3：输出漫反射因子
    // float diff = max(dot(norm, light_dir), 0.0);
    // FragColor = vec4(diff, diff, diff, 1.0);
    // return;

    // 漫反射
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * base_color;

    // 环境光 - 稍微亮一点
    vec3 ambient = 0.4 * base_color;

    vec3 result = ambient + diffuse;

    // Gamma 校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
