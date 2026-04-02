#version 460 core

// 阴影 Pass 的顶点着色器
// 只需要将顶点变换到光源空间，不需要其他计算

layout (location = 0) in vec3 aPos;

uniform mat4 uLightSpaceMatrix;  // 光源的 Projection * View
uniform mat4 uModel;              // 模型矩阵

void main()
{
    // 将顶点变换到光源裁剪空间
    gl_Position = uLightSpaceMatrix * uModel * vec4(aPos, 1.0);
}
