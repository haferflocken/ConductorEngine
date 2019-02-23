$input a_position, a_color0, a_weight
$output v_color0

#include "bgfx_shader.sh"

#if !defined(CONDUCTOR_MAX_BONES)
#	define CONDUCTOR_MAX_BONES 32
#endif // !defined(CONDUCTOR_MAX_BONES)

uniform mat4 u_boneMatrices[CONDUCTOR_MAX_BONES];

void main()
{
	uint8 boneIndex0 = a_weight[0];
	uint8 boneRawWeight0 = a_weight[1];
	uint8 boneIndex1 = a_weight[2];
	uint8 boneRawWeight1 = a_weight[3];

	float boneWeightSum = ((float)boneRawWeight0) + ((float)boneRawWeight1);
	float boneWeight0 = boneRawWeight0 / boneWeightSum;
	float boneWeight1 = boneRawWeight1 / boneWeightSum;

	vec3 bonePos0 = mul(vec4(a_position, 1.0), u_boneMatrices[boneIndex0]);
	vec3 bonePos1 = mul(vec4(a_position, 1.0), u_boneMatrices[boneIndex1]);

	vec3 blendedPosition = vec3(0.0, 0.0, 0.0);
	blendedPos += (bonePos0 * boneWeight0);
	blendedPos += (bonePos1 * boneWeight1);

	gl_Position = mul(vec4(blendedPosition, 1.0), u_viewProj);
	v_color0 = a_color0;
}
