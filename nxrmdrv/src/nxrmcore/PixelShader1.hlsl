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
	float4 DirtyRects[8];
}

float4 NxrmPSMain(NxrmVSOutput Index) : SV_Target0
{
	float4 color = Input.Sample(samp,Index.Tex);

	bool droppixel = true;

	if (Index.TexPos.x > ClipRect.x && Index.TexPos.y < ClipRect.y && Index.TexPos.x < ClipRect.z && Index.TexPos.y > ClipRect.w)
	{
		if (color.r != 1.000000000f || color.g != 1.000000000f || color.b != 1.000000000f)
		{
			color = color * Opacity.a;

			for (int i = 0; i < 8; i++)
			{
				if (DirtyRects[i].x == 0.0f && DirtyRects[i].y == 0.0f && DirtyRects[i].z == 0.0f && DirtyRects[i].w == 0.0f)
				{
					break;
				}

				if (Index.TexPos.x > DirtyRects[i].x && Index.TexPos.y < DirtyRects[i].y && Index.TexPos.x < DirtyRects[i].z && Index.TexPos.y > DirtyRects[i].w)
				{
					droppixel = false;
					break;
				}
			}
		}
	}

	if (droppixel)
	{
		clip(-1);
	}

	return color;
}