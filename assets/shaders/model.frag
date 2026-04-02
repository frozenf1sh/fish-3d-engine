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

    // 简单测试：直接输出法线作为颜色，看看有没有数据
    FragColor = vec4(abs(Normal) * 0.5 + 0.5, 1.0);
}
