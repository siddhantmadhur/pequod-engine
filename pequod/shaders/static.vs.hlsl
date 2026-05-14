cbuffer VS_CAMERA_BUFFER : register(b0)
{
    matrix mWorldViewProj;
    float2 mResolution;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float4 atlas_uv : TEXCOORD1;
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
    output.position = mul(mWorldViewProj, float4(input.position, 1.0));
    output.color = input.color;
    output.uv = input.atlas_uv.xy + input.uv * (input.atlas_uv.zw - input.atlas_uv.xy);
    output.world_normal = input.normal;
    output.world_pos = input.position;
    return output;
}
