$input a_position, a_texcoord0, a_weight
$output v_texcoord0

#include "bgfx_shader.sh"

#if !defined(CONDUCTOR_MAX_BONES)
#	define CONDUCTOR_MAX_BONES 32
#endif // !defined(CONDUCTOR_MAX_BONES)

uniform mat4 u_boneMatrices[CONDUCTOR_MAX_BONES];

void main()
{
	int boneIndex0 = a_weight[0];
	int boneRawWeight0 = a_weight[1];
	int boneIndex1 = a_weight[2];
	int boneRawWeight1 = a_weight[3];

	float boneWeightSum = ((float)boneRawWeight0) + ((float)boneRawWeight1);
	float boneWeight0 = boneRawWeight0 / boneWeightSum;
	float boneWeight1 = boneRawWeight1 / boneWeightSum;

	vec4 bonePos0 = mul(vec4(a_position, 1.0), u_boneMatrices[boneIndex0]);
	vec4 bonePos1 = mul(vec4(a_position, 1.0), u_boneMatrices[boneIndex1]);

	vec4 blendedPosition = (bonePos0 * boneWeight0) + (bonePos1 * boneWeight1);

	gl_Position = mul(blendedPosition, u_viewProj);
	v_texcoord0 = a_texcoord0;
}
