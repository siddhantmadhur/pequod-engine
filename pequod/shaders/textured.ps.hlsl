Texture2D<float4> diffuse_texture : register(t0);
Texture2D<float> shadow_map : register(t1);
SamplerState texture_sampler : register(s0);
SamplerComparisonState shadow_cmp_sampler : register(s1);

cbuffer PS_LIGHT_BUFFER : register(b2)
{
    float3 sun_direction;
    float  sun_intensity;
    float3 sun_color;
    float  saturation;
    float3 ambient_color;
    float  _pad1;
    matrix shadow_view_proj;
};

struct PSInput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float3 world_normal : TEXCOORD1;
    float3 world_pos : TEXCOORD2;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

float SampleShadow(float3 world_pos)
{
    float4 lp = mul(shadow_view_proj, float4(world_pos, 1.0));
    float3 ndc = lp.xyz / lp.w;
    // Reject samples outside the shadow frustum (treated as lit).
    if (any(abs(ndc.xy) > 1.0) || ndc.z < 0.0 || ndc.z > 1.0)
        return 1.0;
    float2 uv = ndc.xy * float2(0.5, -0.5) + 0.5;
    // 2x2 PCF via comparison sampler. Returns 1.0 = lit, 0.0 = fully shadowed.
    return shadow_map.SampleCmpLevelZero(shadow_cmp_sampler, uv, ndc.z);
}

PSOutput Main(PSInput input)
{
    PSOutput output = (PSOutput) 0;

    float4 albedo = diffuse_texture.Sample(texture_sampler, input.uv) * input.color;

    // Standard Lambert — hard terminator gives strong cartoon shadows.
    float3 N = normalize(input.world_normal);
    float3 L = -normalize(sun_direction);
    float diffuse = saturate(dot(N, L));

    // Shadow map: pixels occluded from the sun fall to ambient only.
    float shadow = SampleShadow(input.world_pos);
    diffuse *= shadow;

    float3 lit = albedo.rgb * (ambient_color + sun_color * sun_intensity * diffuse);

    // Saturation adjustment around perceptual luma. 0 = grayscale, 1 = source,
    // >1 = punchier color. Done in linear space before gamma.
    float luma = dot(lit, float3(0.2126, 0.7152, 0.0722));
    lit = lerp(float3(luma, luma, luma), lit, saturation);

    // Gamma-correct on output since we render straight to a UNORM backbuffer.
    float3 srgb = pow(saturate(lit), 1.0 / 2.2);
    output.color = float4(srgb, albedo.a);
    return output;
}
