//---------------------------------------------------
// Global Variables
//---------------------------------------------------
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = false; //default
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
};

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
	float2 TextureUV : TEXCOORD;
};

//---------------------------------------------------
// Vertex Shader
//---------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.TextureUV = input.TextureUV;
	return output;
}

//---------------------------------------------------
// Pixel Shader
//---------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(gSamPoint, input.TextureUV);
}

float4 PS_LIN(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(gSamLinear, input.TextureUV);
}

float4 PS_ANI(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(gSamAnisotropic, input.TextureUV);
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