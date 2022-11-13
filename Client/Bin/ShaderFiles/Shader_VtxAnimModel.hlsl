#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

float g_fDissolveAcc;
bool g_bNormalTex;
struct tagBoneMatrices
{
	matrix		BoneMatrix[256];
};

tagBoneMatrices		g_BoneMatrices;

texture2D	g_DiffuseTexture;
texture2D	g_NormalTexture;
texture2D	g_DissolveTexture;

sampler DefaultSampler = sampler_state {

	filter = min_mag_mip_linear;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float3		vTangent : TANGENT;
	uint4		vBlendIndex : BLENDINDEX;
	float4		vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	float		fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

	float4x4	BoneMatrix = g_BoneMatrices.BoneMatrix[In.vBlendIndex.x] * In.vBlendWeight.x +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.y] * In.vBlendWeight.y +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.z] * In.vBlendWeight.z +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.w] * fWeightW;

	vector		vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
	vector		vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
	vector		vTangent = mul(vector(In.vTangent, 0.f), BoneMatrix);

	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));

	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (g_bNormalTex)
	{
		Out.vDiffuse = (vector)1.f;

		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
		vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

		float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

		float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

		vNormal = normalize(mul(vNormal, WorldMatrix));

		Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
	}

	else
	{
		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

		Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);

	}

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Pattern_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (g_bNormalTex)
	{
		Out.vDiffuse = (vector)1.f;

		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
		vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

		float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

		float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

		vNormal = normalize(mul(vNormal, WorldMatrix));

		Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.b = 1.f;
	}

	else
	{
		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

		Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.b = 1.f;
	}

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Hit_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (g_bNormalTex)
	{
		Out.vDiffuse = (vector)1.f;

		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
		vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

		float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

		float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

		vNormal = normalize(mul(vNormal, WorldMatrix));

		Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 1.f;
	}

	else
	{
		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

		Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 1.f;

	}

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Finish_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (g_bNormalTex)
	{
		Out.vDiffuse = (vector)1.f;

		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
		vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

		float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

		float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

		vNormal = normalize(mul(vNormal, WorldMatrix));

		Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 0.8f;
	}

	else
	{
		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

		Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 0.8f;

	}

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT PlayerVoid_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);


	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Dissolve_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	if (g_bNormalTex)
	{
		Out.vDiffuse = (vector)1.f;

		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
		vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

		float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

		float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

		vNormal = normalize(mul(vNormal, WorldMatrix));

		Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 0.8f;
	}

	else
	{
		Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

		Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
		Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.0f, 0.0f);
		Out.vDiffuse.r = 0.8f;

	}

	float fDissolve = length(g_DissolveTexture.Sample(DefaultSampler, In.vTexUV));
	fDissolve = smoothstep(0.f, 4.f, fDissolve);

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (fDissolve <= g_fDissolveAcc)
		discard;

	if (fDissolve <= g_fDissolveAcc + 0.03f)
	{
		Out.vDiffuse.r = 1.f;
		Out.vDiffuse.g = 1.f;
		Out.vDiffuse.b *= 1.f - (g_fDissolveAcc - fDissolve);
	}

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

technique11 DefaultTechnique
{
	pass DefaultPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}

	pass PatternPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 Pattern_MAIN();
	}

	pass HitPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 Hit_MAIN();
	}

	pass FinishPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 Finish_MAIN();
	}

	pass PlayerVoidPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PlayerVoid_MAIN();
	}

	pass DissolvePass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 Dissolve_MAIN();
	}

}