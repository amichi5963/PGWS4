#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1, -1, 1));
	float brightness = dot(-light, input.normal.xyz);
	float2 normalUV = input.normal.xy * float2(0.5, -0.5) + 0.5;
	float2 sphereMapUV = input.vnormal.xy * float2(0.5, -0.5) + 0.5;
	float4 color = tex.Sample(smp, input.uv);
	return float4(brightness, brightness, brightness, 1) // �P�x
		* diffuse // �f�B�t���[�Y�F
		* color // �e�N�X�`���J���[
		* sph.Sample(smp, sphereMapUV) // �X�t�B�A�}�b�v(��Z)
		+ spa.Sample(smp, sphereMapUV) // �X�t�B�A�}�b�v(���Z)
		+ float4(color * ambient, 0);
}
