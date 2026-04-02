#version 460 core

// 阴影 Pass 的片段着色器
// 实际上不需要输出任何颜色，只需要写入深度缓冲
// 但我们必须提供一个片段着色器，即使它是空的

void main()
{
    // 不需要做任何事，GLFW 会自动写入深度值
    // 对于高级阴影技术（如 Variance Shadow Mapping），这里可以输出深度
}
