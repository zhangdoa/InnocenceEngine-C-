// shadertype=hlsl
#include "common.hlsl"
#define BLOCK_SIZE 16

struct ComputeInputType
{
	uint3 groupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
	uint3 groupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
	uint3 dispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
	uint  groupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

cbuffer dispatchParamsCBuffer : register(b0)
{
	uint3 numThreadGroups;
	uint dispatchParamsCBuffer_padding1;
	uint3 numThreads;
	uint  dispatchParamsCBuffer_padding2;
}

cbuffer skyCBuffer : register(b1)
{
	matrix p_inv;
	matrix v_inv;
	float2 viewportSize;
	float2 padding1;
};

RWStructuredBuffer<Frustum> out_Frustums : register(u0);

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeInputType input)
{
	// View space eye position is always at the origin.
	const float3 eyePos = float3(0, 0, 0);

	// Compute the 4 corner points on the far clipping plane to use as the 
	// frustum vertices.
	float4 screenSpace[4];
	// Top left point
	screenSpace[0] = float4(input.dispatchThreadID.xy * BLOCK_SIZE, -1.0f, 1.0f);
	// Top right point
	screenSpace[1] = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y) * BLOCK_SIZE, -1.0f, 1.0f);
	// Bottom left point
	screenSpace[2] = float4(float2(input.dispatchThreadID.x, input.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);
	// Bottom right point
	screenSpace[3] = float4(float2(input.dispatchThreadID.x + 1, input.dispatchThreadID.y + 1) * BLOCK_SIZE, -1.0f, 1.0f);

	float3 viewSpace[4];
	// Now convert the screen space points to view space
	for (int i = 0; i < 4; i++)
	{
		viewSpace[i] = ScreenToView(screenSpace[i], viewportSize, p_inv).xyz;
	}

	// Now build the frustum planes from the view space points
	Frustum frustum;

	// Left plane
	frustum.planes[0] = ComputePlane(eyePos, viewSpace[2], viewSpace[0]);
	// Right plane
	frustum.planes[1] = ComputePlane(eyePos, viewSpace[1], viewSpace[3]);
	// Top plane
	frustum.planes[2] = ComputePlane(eyePos, viewSpace[0], viewSpace[1]);
	// Bottom plane
	frustum.planes[3] = ComputePlane(eyePos, viewSpace[3], viewSpace[2]);

	// Store the computed frustum in global memory (if our thread ID is in bounds of the grid).
	if (input.dispatchThreadID.x < numThreads.x && input.dispatchThreadID.y < numThreads.y)
	{
		uint index = input.dispatchThreadID.x + (input.dispatchThreadID.y * numThreads.x);
		out_Frustums[index] = frustum;
	}
}