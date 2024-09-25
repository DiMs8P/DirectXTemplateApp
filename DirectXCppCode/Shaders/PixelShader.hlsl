//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float3 Normal: NORMAL;
	float4 Color : COLOR0;
	float2 TexCoords : TEXCOORD0;
};
Texture2D  gTexture : register(t0);
SamplerState TextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
};
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( VS_OUTPUT input ) : SV_Target
{
	float2 texturecoor;
	texturecoor[0] = input.TexCoords[0];
	texturecoor[1] = 1-input.TexCoords[1];
	return gTexture.Sample(TextureSampler, texturecoor);
}
