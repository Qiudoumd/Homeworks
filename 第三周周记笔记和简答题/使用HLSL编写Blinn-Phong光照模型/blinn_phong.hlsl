//AI模型生成的模板
// 顶点着色器输入结构
struct VS_INPUT
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

// 顶点着色器输出结构，也是像素着色器输入结构
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 normal : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 lightDir : TEXCOORD2;
};

// 全局常量缓冲区
cbuffer ConstantBuffer
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float3 lightPosition;
    float3 cameraPosition;
};

// 顶点着色器
VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output;

    // 计算世界空间位置
    float4 worldPos = mul(input.position, world);

    // 变换顶点到裁剪空间
    output.position = mul(worldPos, view);
    output.position = mul(output.position, projection);

    // 计算世界空间法线
    output.normal = mul(input.normal, (float3x3)world);
    output.normal = normalize(output.normal);

    // 计算视线方向
    output.viewDir = normalize(cameraPosition - worldPos.xyz);

    // 计算光照方向
    output.lightDir = normalize(lightPosition - worldPos.xyz);

    return output;
}

// 像素着色器
float4 PS_Main(VS_OUTPUT input) : SV_TARGET
{
    // 环境光
    float3 ambient = float3(0.2, 0.2, 0.2);

    // 漫反射
    float diff = max(dot(input.normal, input.lightDir), 0.0);
    float3 diffuse = float3(1.0, 1.0, 1.0) * diff;

    // Blinn-Phong高光
    float3 halfDir = normalize(input.lightDir + input.viewDir);
    float spec = pow(max(dot(input.normal, halfDir), 0.0), 32.0);
    float3 specular = float3(1.0, 1.0, 1.0) * spec;

    // 最终颜色
    float3 finalColor = ambient + diffuse + specular;

    return float4(finalColor, 1.0);
}    



//根据07项目和AI模板改编得来

cbuffer VSConstantBuffer : register(b0)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
    matrix g_WorldInvTranspose;
    float3 lightPosition1;
    float3 cameraPosition1;
}

struct VertexIn
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION; // 在世界中的位置
    float3 normalW : NORMAL; // 法向量在世界中的方向
    float4 color : COLOR;
    float3 viewDir : VIEW;
    float3 lightDir : LIGHT;
};

VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World); //世界空间

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    vOut.color = vIn.color; // 这里alpha通道的值默认为1.0
    // 计算视线方向
    vOut.viewDir = normalize(cameraPosition - posW.xyz);

    // 计算光照方向
    vOut.lightDir = normalize(lightPosition - posW.xyz);

    return vOut;
}

float4 PS(VertexOut vIn) : SV_TARGET
{
    // 环境光
    float3 ambient = float3(0.2, 0.2, 0.2);

    // 漫反射
    float diff = max(dot(vIn.normalW, vIn.lightDir), 0.0);
    float3 diffuse = float3(1.0, 1.0, 1.0) * diff;

    // Blinn-Phong高光
    float3 halfDir = normalize(vIn.lightDir + vIn.viewDir);
    float spec = pow(max(dot(vIn.normalW, halfDir), 0.0), 32.0);
    float3 specular = float3(1.0, 1.0, 1.0) * spec;

    // 最终颜色
    float3 finalColor = ambient + diffuse + specular;

    return float4(finalColor, 1.0);
}