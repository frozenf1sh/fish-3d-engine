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

uniform mat4 uLightSpaceMatrix;
uniform sampler2D uShadowMap;

uniform sampler2D uBaseColorTexture;

out vec4 FragColor;

//==========================================================================
// Shadow Bias（阴影偏移）的数学原理
//==========================================================================
//
// 问题：Shadow Acne（阴影痤疮）
//   - 阴影贴图的分辨率有限，多个片段可能对应阴影贴图中的同一个纹素
//   - 当光线与表面几乎平行时，小的深度误差会导致自遮挡
//
// 解决方案：Shadow Bias
//   - 基于表面法线和光线方向的夹角，动态计算偏移量
//   - 当光线垂直于表面时（cos=1），使用较小的偏移
//   - 当光线与表面几乎平行时（cos→0），使用较大的偏移
//
// 公式：bias = max(bias_max * (1.0 - dot(N, L)), bias_min)
//   - N: 表面法线
//   - L: 光线方向
//   - bias_min: 最小偏移（避免垂直时仍有 acne）
//   - bias_max: 最大偏移（避免掠射角时的 acne）
//
//==========================================================================
float calculate_shadow_bias(vec3 normal, vec3 light_dir)
{
    float bias_min = 0.005;
    float bias_max = 0.05;
    float cos_theta = max(dot(normal, light_dir), 0.0);
    return max(bias_max * (1.0 - cos_theta), bias_min);
}

//==========================================================================
// PCF (Percentage-Closer Filtering) 软阴影
//==========================================================================
//
// 原理：
//   - 不只是比较单个深度值，而是比较阴影贴图中该点周围的多个采样点
//   - 统计有多少比例的采样点在阴影中
//   - 结果是 0.0（完全照亮）到 1.0（完全阴影）之间的值
//
// 为什么叫 "Percentage-Closer"：
//   - 计算的是"比当前片段深度更近的采样点的百分比"
//   - 这个百分比就是阴影的程度
//
// 本实现：4x4 采样网格
//   - 在阴影贴图中取 16 个点（从 -2 到 +1）
//   - 每个点偏移 1/shadow_map_size 的距离
//
//==========================================================================
float calculate_shadow_pcf(vec4 frag_pos_light_space, vec3 normal, vec3 light_dir)
{
    // 1. 透视除法：将裁剪空间坐标转换到 NDC [-1, 1]
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;

    // 2. 转换到纹理坐标空间 [0, 1]
    // NDC 的 [-1, 1] 范围要变成纹理的 [0, 1] 范围
    proj_coords = proj_coords * 0.5 + 0.5;

    // 3. 如果超出阴影贴图范围，认为完全照亮（无阴影）
    if (proj_coords.z > 1.0) {
        return 0.0;
    }

    // 4. 计算 Shadow Bias
    float bias = calculate_shadow_bias(normal, light_dir);

    // 5. PCF 采样：4x4 网格，共 16 个采样点
    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(uShadowMap, 0);

    // 采样从 -2 到 +1，这样中心点周围有更平衡的分布
    for (int x = -2; x <= 1; ++x)
    {
        for (int y = -2; y <= 1; ++y)
        {
            // 获取相邻纹素的深度值
            float pcf_depth = texture(uShadowMap, proj_coords.xy + vec2(x, y) * texel_size).r;

            // 比较：如果当前片元深度（减去 bias）大于阴影贴图中的深度，
            // 说明该片元在阴影中
            shadow += (proj_coords.z - bias > pcf_depth) ? 1.0 : 0.0;
        }
    }

    // 6. 取平均值：16个采样点 / 16
    shadow /= 16.0;

    return shadow;
}

void main()
{
    // 先获取纹理颜色
    vec3 base_color = uHasTexture ? texture(uBaseColorTexture, TexCoord).rgb : uBaseColor;

    // 计算光照（Phong模型）
    vec3 norm = normalize(Normal);
    vec3 light_dir = normalize(uLightDir);
    vec3 view_dir = normalize(uViewPos - FragPos);

    // 环境光
    float ambient_strength = 0.15;
    vec3 ambient = ambient_strength * uLightColor * base_color;

    // 漫反射
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * uLightColor * base_color;

    // 镜面反射 (Blinn-Phong)
    float specular_strength = 0.5 * (1.0 - uRoughness);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(norm, halfway_dir), 0.0), 32.0);
    vec3 specular = specular_strength * spec * uLightColor;

    // 计算阴影
    vec4 frag_pos_light_space = uLightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = calculate_shadow_pcf(frag_pos_light_space, norm, light_dir);

    // 最终颜色：阴影部分只保留环境光
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    // Gamma 校正
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
