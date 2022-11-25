#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix, g_LightViewMatrix, g_LightProjMatrix;
matrix g_WorldMatrixInverse, g_ViewMatrixInverse;
vector g_vCamPos;
float g_fDissolveAcc;
float g_fGolemPattern;
float g_fOutLinePower;
float g_fWinSizeX;
float g_fWinSizeY;

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
	float4		vWorldPos : TEXCOORD2;
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

	vector		vWorld = mul(vPosition, g_WorldMatrix);

	Out.vPosition = mul(vPosition, matWVP);
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));

	Out.vTexUV = In.vTexUV;
	Out.vProjPos = Out.vPosition;

	//Out.vTestNormal = normalize(mul(normalize(vNormal), g_WorldMatrix));

	Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);
	

	return Out;
}

VS_OUT OutLine_MAIN(VS_IN In)
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
	/*Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));

	Out.vTexUV = In.vTexUV;*/
	Out.vProjPos = Out.vPosition;
	Out.vWorldPos = mul(vector(In.vPosition, 1.f), g_WorldMatrix);

	
	matrix matWVInverse;
	matWVInverse = mul(g_WorldMatrixInverse, g_ViewMatrixInverse);

	vector tNormal = normalize(mul(vNormal, matWVInverse));

	float2 offset = mul(g_ProjMatrix, tNormal.xy);
	offset.x /= g_fWinSizeX;
	offset.y /= g_fWinSizeY;
	Out.vPosition.xy += offset * Out.vPosition.z * g_fOutLinePower;

	return Out;
}



struct VS_IN_SHADOW
{
	float3      vPosition : POSITION;
	float3      vNormal : NORMAL;
	float2      vTexUV : TEXCOORD0;
	float3      vTangent : TANGENT;
	uint4      vBlendIndex : BLENDINDEX;
	float4      vBlendWeight : BLENDWEIGHT;
};

struct VS_OUT_SHADOW
{
	float4         vPosition : SV_POSITION;
	float4         vProjPos : TEXCOORD0;
};

VS_OUT_SHADOW VS_MAIN_SHADOW(VS_IN_SHADOW In)
{
	VS_OUT_SHADOW      Out = (VS_OUT_SHADOW)0;

	matrix      matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_LightViewMatrix);
	matWVP = mul(matWV, g_LightProjMatrix);

	float      fWeightW = 1.f - (In.vBlendWeight.x + In.vBlendWeight.y + In.vBlendWeight.z);

	float4x4   BoneMatrix = g_BoneMatrices.BoneMatrix[In.vBlendIndex.x] * In.vBlendWeight.x +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.y] * In.vBlendWeight.y +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.z] * In.vBlendWeight.z +
		g_BoneMatrices.BoneMatrix[In.vBlendIndex.w] * fWeightW;

	vector      vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
	vector      vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);


	Out.vPosition = mul(vPosition, matWVP);
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN_SHADOW
{
	float4         vPosition : SV_POSITION;
	float4         vProjPos : TEXCOORD0;
};

struct PS_OUT_SHADOW
{
	float4         vLightDepth : SV_TARGET0;
};

PS_OUT_SHADOW PS_MAIN_SHADOW(PS_IN_SHADOW In)
{
	PS_OUT_SHADOW      Out = (PS_OUT_SHADOW)0;


	// 빛에서 바라본 
	//               0~ 300    0~1
	Out.vLightDepth.r = In.vProjPos.w / 300.f;
	Out.vLightDepth.g = 0.5f;
	// Out.vLightDepth.r = 1.f;


	Out.vLightDepth.a = 1.f;


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
	float4		vWorldPos : TEXCOORD2;
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

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
	//림라이트
	//float3 rimColor = float3(1.f, 0.f, 0.f);

	//if (rim <= 0.3f)
	//{
	//	//Out.vDiffuse.r *= rim;
	//	Out.vDiffuse.r = 1.f;
	//	Out.vDiffuse.gb = 0.f;
	//}
	//림라이트
	

	/*float rimWidth = 0.2f;
	float RimLightColor = smoothstep(1.0f - rimWidth, 1.0f, 1 - max(0, dot(normalize(In.vNormal), vCameraPos)));

	float3 rimColor = float3(1.f, 0.f, 0.f);

	Out.vDiffuse.rgb += rimColor * RimLightColor;*/

	
}

PS_OUT NPS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	
	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Pattern_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.b = 1.f;

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT NPattern_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.b = 1.f;

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Hit_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;
	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse.gb = 0.f;

	return Out;
}

PS_OUT Finish_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.r = 0.8f;

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT NFinish_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.r = 0.8f;

	if (0 == Out.vDiffuse.a)
		discard;

	return Out;
}

PS_OUT Dissolve_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.r = 0.8f;

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

PS_OUT NDissolve_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);
	Out.vDiffuse.r = 0.8f;

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

PS_OUT GolemPattern_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = (vector)1.f;

	Out.vDiffuse = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);
	vector		vNormalDesc = g_NormalTexture.Sample(DefaultSampler, In.vTexUV);

	float3		vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3	WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	vNormal = normalize(mul(vNormal, WorldMatrix));

	Out.vNormal = vector(vNormal.xyz * 0.5f + 0.5f, 0.f);

	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 300.0f, 0.5f, 0.0f);

	Out.vDiffuse.r += g_fGolemPattern;
	Out.vDiffuse.gb -= min(0.6f, g_fGolemPattern);

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

	pass NDefaultPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 NPS_MAIN();
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

	pass NPatternPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 NPattern_MAIN();
	}

	pass HitPass
	{
		SetRasterizerState(RS_OutLine);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 OutLine_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 Hit_MAIN();
	}

	pass NHitPass
	{
		SetRasterizerState(RS_OutLine);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 OutLine_MAIN();
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

	pass NFinishPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 NFinish_MAIN();
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

	pass NDissolvePass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 NDissolve_MAIN();
	}

	pass GolemPattern
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 GolemPattern_MAIN();
	}

	pass Shadow_Depth
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Shadow, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN_SHADOW();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_MAIN_SHADOW();
	}

}