#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1, -1, 1));
	float brightness = dot(-light, input.normal.xyz);
	float2 normalUV = input.normal.xy * float2(0.5, -0.5) + 0.5;
	return float4(brightness, brightness, brightness, 1) // �P�x
		* diffuse // �f�B�t���[�Y�F
		* tex.Sample(smp, input.uv) // �e�N�X�`���J���[
		* sph.Sample(smp, normalUV); // �X�t�B�A�}�b�v( ��Z)
}
