struct NxrmVSInput
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct NxrmVSOutput
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 TexPos : TEXCOORD1;
};

cbuffer cbNxrmTex2Screen : register(b0)
{
	float4 Tex2Screen;
}

NxrmVSOutput NxrmVSMain(NxrmVSInput Input)
{
	NxrmVSOutput Output;
	Output.Pos = Input.Pos;
	Output.Tex = float2(Input.Tex.x * Tex2Screen.x, Input.Tex.y * Tex2Screen.y);
	Output.TexPos = Input.Pos.xy;

	return Output;
}