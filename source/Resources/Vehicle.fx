//---------------------------------------------------
// Global Variables
//---------------------------------------------------
float gPI = 3.14159265358979323846f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld : World;
float4x4 gInvView : InverseView;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossMap : GlossinessMap;

RasterizerState gRasterizerState {};
BlendState gBlendState {};
DepthStencilState gDepthStencilState {};

SamplerState gSamPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; // Mirror, Clamp, Border or Wrap
	AddressV = Wrap; // Mirror, Clamp, Border or Wrap
};

SamplerState gSamLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap; // Mirror, Clamp, Border or Wrap
	AddressV = Wrap; // Mirror, Clamp, Border or Wrap
};

SamplerState gSamAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap; // Mirror, Clamp, Border or Wrap
	AddressV = Wrap; // Mirror, Clamp, Border or Wrap
};

//---------------------------------------------------
// Input/Output Structs
//---------------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TextureUV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TextureUV : TEXCOORD;
};

//---------------------------------------------------
// Vertex Shader
//---------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorld);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	output.TextureUV = input.TextureUV;
	return output;
}

//---------------------------------------------------
// BRDF
//---------------------------------------------------
float3 Lambert(float kd, float3 cd)
{
	float temp = kd / gPI;
	return float3(cd.x * temp, cd.y * temp, cd.z * temp);
}

float3 Phong(float3 ks, float exp, float3 l, float3 v, float3 n)
{
	float3 result = reflect(l, n);
	float dotProduct = saturate(dot(result, v));
	return ks * pow(dotProduct, exp);
}

//---------------------------------------------------
// Pixel Shader
//---------------------------------------------------
float3 Shading(VS_OUTPUT input, SamplerState sam)
{
	//final color
	float3 finalColor = float3(0.025f, 0.025f, 0.025f);

	//view direction
	float3 viewDirection = normalize(float3(input.WorldPosition.xyz) - float3(gInvView[3].xyz));

	//normal map
	float3 binormal = cross(input.Normal, input.Tangent);
	float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.f), float4(binormal, 0.f), float4(input.Normal, 0.f), float4(0.f, 0.f, 0.f, 1.f));
	float4 sampledColor = gNormalMap.Sample(sam, input.TextureUV);
	float3 partialColor = 2.f * sampledColor.rgb - float3(1.f, 1.f, 1.f);
	float3 normalResult = mul(float4(partialColor, 0.0f), tangentSpaceAxis);

	//Observed area (lambert cosine law)
	float dotProduct = saturate(dot(normalResult, -gLightDirection));

	//Sampled Textures
	float4 sampledDiffuse = gDiffuseMap.Sample(sam, input.TextureUV);
	float4 sampledSpecular = gSpecularMap.Sample(sam, input.TextureUV);
	float4 sampledGlossiness = gGlossMap.Sample(sam, input.TextureUV);

	finalColor += Lambert(gLightIntensity, sampledDiffuse.rgb) * dotProduct;
	finalColor += Phong(sampledSpecular.rgb, gShininess * sampledGlossiness.r, -gLightDirection, viewDirection, normalResult) * dotProduct;

	return finalColor;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 finalColor = Shading(input, gSamPoint);
	return float4(finalColor, 1.f);
}

float4 PS_LIN(VS_OUTPUT input) : SV_TARGET
{
	float3 finalColor = Shading(input, gSamLinear);
	return float4(finalColor, 1.f);
}

float4 PS_ANI(VS_OUTPUT input) : SV_TARGET
{
	float3 finalColor = Shading(input, gSamAnisotropic);
	return float4(finalColor, 1.f);
}

//---------------------------------------------------
// Technique
//---------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_LIN()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ANI()));
	}
}