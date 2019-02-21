$input a_position, a_color0, a_weight
$output v_color0

#include "bgfx_shader.sh"

#if !defined(CONDUCTOR_MAX_BONES)
#	define CONDUCTOR_MAX_BONES 32
#endif // !defined(CONDUCTOR_MAX_BONES)

#if !defined(CONDUCTOR_MAX_WEIGHT_GROUPS)
#	define CONDUCTOR_MAX_WEIGHT_GROUPS 32
#endif // !defined(CONDUCTOR_MAX_WEIGHT_GROUPS)

uniform mat4 u_boneMatrices[CONDUCTOR_MAX_BONES];
// bgfx doesn't support a pure float uniform, so they're passed in as groups of 4.
uniform vec4 u_weightGroups[(CONDUCTOR_MAX_BONES * CONDUCTOR_MAX_WEIGHT_GROUPS + 3) / 4];

void main()
{
	vec3 blendedPosition = vec3(0.0, 0.0, 0.0);

	int weightGroupStart = a_weight * CONDUCTOR_MAX_BONES;
	for (int i = 0; i < CONDUCTOR_MAX_BONES; ++i)
	{
		int vecIndex = (weightGroupStart + i) / 4;
		int indexInVec = (weightGroupStart + i) % 4;

		float boneWeight = u_weightGroups[vecIndex][indexInVec];
		vec3 bonePos = mul(vec4(a_position, 1.0), u_boneMatrices[i]);
		blendedPos += (bonePos * boneWeight);
	}

	gl_Position = mul(vec4(blendedPosition, 1.0), u_viewProj);
	v_color0 = a_color0;
}
