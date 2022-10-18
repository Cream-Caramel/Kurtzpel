#include "Client_Shader_Defines.hpp"
matrix		g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
float g_fCoolTime;
float g_fCoolTimeOn;
float g_fHitColor;
float g_fNowPlayerHp;
float g_fPrePlayerHp;
float g_fPlayerMp;
float g_fExGauge;
float4 g_fTileColor;
float4 g_fTrailColor;

sampler DefaultSampler = sampler_state {

	filter = min_mag_mip_linear;
	/*minfilter = linear;
	magfilter = linear;
	mipfilter = linear;*/
};

struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};


VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vTexUV = In.vTexUV;
	
	return Out;
}
VS_OUT VS_MAIN_Trail(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWVP = mul(g_ViewMatrix, g_ProjMatrix);

	Out.vPosition = mul(float4(In.vPosition, 1.f), matWVP);
	Out.vTexUV = In.vTexUV;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	float4		vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;	
}

PS_OUT PS_Trail(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_fTrailColor;

	Out.vColor.a = 1.f - In.vTexUV.y;

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT CoolTime_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (In.vTexUV.y > 1.f - g_fCoolTime)
	{
		Out.vColor.r = 0.7f;
		Out.vColor.gb = 0.1f;
	}

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT CoolTimeOn_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);


	Out.vColor.r = g_fCoolTimeOn;
	Out.vColor.gb = g_fCoolTimeOn;


	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT ImpossibleUse_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor.r = 0.5f;
	Out.vColor.gb = 0.1f;

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT KeyDown_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor.r = 0.f;
	Out.vColor.gb = 0.f;

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT HpBarFrame_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor.r = 0.f;
	Out.vColor.gb = 0.f;

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}



PS_OUT HpBar_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (In.vTexUV.x < g_fNowPlayerHp)
	{
		Out.vColor.r = 1.f;
		Out.vColor.gb = 1.f;
	}
	else if (In.vTexUV.x < g_fPrePlayerHp)
	{
		Out.vColor.r = 1.f;
		Out.vColor.gb = 0.f;
	}

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT BossHpBar_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (In.vTexUV.x < g_fNowPlayerHp)
	{
		Out.vColor.r = 1.f;
		Out.vColor.gb = 0.f;
	}
	else if (In.vTexUV.x < g_fPrePlayerHp)
	{
		Out.vColor.rg = 1.f;
		Out.vColor.b = 0.f;
	}

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT ExGauge_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	Out.vColor.r = 1.f;
	Out.vColor.gb = g_fExGauge;

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT MpBar_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (In.vTexUV.x < g_fPlayerMp)
	{
		Out.vColor.rb = 0.64f;
		Out.vColor.g = 0.29f;
	}
	else if (In.vTexUV.x > g_fPlayerMp)
	{
		Out.vColor.r = 0.f;
		Out.vColor.gb = 0.f;
	}

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT BossMpBar_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	if (In.vTexUV.x < g_fPlayerMp)
	{
		Out.vColor.r = 1.f;
		Out.vColor.g = 0.5f;
		Out.vColor.b = 0.f;
	}
	else if (In.vTexUV.x > g_fPlayerMp)
	{
		Out.vColor.r = 0.f;
		Out.vColor.gb = 0.f;
	}

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

	return Out;
}

PS_OUT SkillIcon_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vColor = g_DiffuseTexture.Sample(DefaultSampler, In.vTexUV);

	clip(Out.vColor.a <= 0.f ? -1.f : 1.f);

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

	pass CoolTimePass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 CoolTime_MAIN();
	}

	pass ImpossibleUsePass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 ImpossibleUse_MAIN();
	}

	pass CoolTimeOnPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 CoolTimeOn_MAIN();
	}

	pass KeyDownPass
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 KeyDown_MAIN();
	}

	pass HPBarFrame
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 HpBarFrame_MAIN();
	}

	pass HPBar
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 HpBar_MAIN();
	}

	pass BossHPBar
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 BossHpBar_MAIN();
	}

	pass ExGauge
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 ExGauge_MAIN();
	}

	pass MPBar
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 MpBar_MAIN();
	}

	pass BossMPBar
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 BossMpBar_MAIN();
	}

	pass SkillIcon
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 SkillIcon_MAIN();
	}

	pass Trail
	{
		SetRasterizerState(RS_Trail);
		SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
		VertexShader = compile vs_5_0 VS_MAIN_Trail();
		GeometryShader = NULL;
		PixelShader = compile ps_5_0 PS_Trail();
	}


	
}