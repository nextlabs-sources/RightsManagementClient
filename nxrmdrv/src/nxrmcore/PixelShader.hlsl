struct NxrmVSOutput
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 TexPos : TEXCOORD1;
};

SamplerState samp : register(s0);
Texture2D<float4> Input : register(t0);

cbuffer cbNxrmClipRect : register(b1)
{
	float4 ClipRect;
	float4 Opacity;
}

float4 NxrmPSMain(NxrmVSOutput Index) : SV_Target0
{
	float4 color = Input.Sample(samp,Index.Tex);

	if (Index.TexPos.x > ClipRect.x && Index.TexPos.y < ClipRect.y && Index.TexPos.x < ClipRect.z && Index.TexPos.y > ClipRect.w)
	{
		if (color.r == 1.000000000f && color.g == 1.000000000f && color.b == 1.000000000f)
		{
			clip(-1);
		}
	}
	else
	{
		clip(-1);
	}
	
	return color;
}