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
    // 测试1：先显示纹理，不做任何光照
    if (uHasTexture) {
        FragColor = texture(uBaseColorTexture, TexCoord);
    } else {
        FragColor = vec4(uBaseColor, 1.0);
    }

    // 测试2：再试试显示光源方向作为颜色
    // FragColor = vec4(abs(uLightDir) * 0.5 + 0.5, 1.0);
}
