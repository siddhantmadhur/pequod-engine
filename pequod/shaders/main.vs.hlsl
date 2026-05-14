cbuffer VS_CAMERA_BUFFER : register(b0)
{
    matrix mWorldViewProj;
    float2 mResolution;
};

cbuffer VS_MODEL_BUFFER : register(b1)
{
    float3 scale;
    float opacity;
    matrix model;
    float4 atlas_uv;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 world_normal : TEXCOORD1;
    float3 world_pos : TEXCOORD2;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    float3 scaled_position = input.position * scale;
    float4 world_pos = mul(model, float4(scaled_position, 1.0));

    output.position = mul(mWorldViewProj, world_pos);
    output.color = float4(input.color, opacity);
    output.uv = atlas_uv.xy + input.uv * (atlas_uv.zw - atlas_uv.xy);
    output.world_normal = mul((float3x3) model, input.normal);
    output.world_pos = world_pos.xyz;
    return output;
}
