//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix View;
	matrix Projection;
	matrix WorldMatrix;
}
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float3 Normal: NORMAL;
	float4 Color : COLOR0;
	float2 TexCoords : TEXCOORD0;
};
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( float4 InPos : POSITION, float4 InColor : COLOR , float3 InNormal : NORMAL, float2 InTexCoords : TEXCOORD0)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = InPos;
	output.Pos = mul( output.Pos, WorldMatrix );
	output.Pos = mul( output.Pos, View );
	output.Pos = mul( output.Pos, Projection );

	output.Normal = InNormal;
	output.Color = InColor;
	output.TexCoords = InTexCoords;
	return output;
}
