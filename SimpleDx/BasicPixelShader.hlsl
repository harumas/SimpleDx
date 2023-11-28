struct Input {
	float4 svpos:SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 BasicPS(Input input ) : SV_TARGET{
	//return float4((float2(0,1)+ input.pos.xy)*0.5f,1,1);
	return float4(input.uv, 1, 1);
}