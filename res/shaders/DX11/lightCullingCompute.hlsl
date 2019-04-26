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

cbuffer cameraCBuffer : register(b1)
{
	matrix cam_p_original;
	matrix cam_p_jittered;
	matrix cam_r;
	matrix cam_t;
	matrix cam_r_prev;
	matrix cam_t_prev;
	float4 cam_globalPos;
	float cam_WHRatio;
};

cbuffer skyCBuffer : register(b2)
{
	matrix p_inv;
	matrix v_inv;
	float2 viewportSize;
	float2 padding1;
};

cbuffer pointLightCBuffer : register(b3)
{
	pointLight pointLights[NR_POINT_LIGHTS];
};

Texture2D DepthTextureVS : register(t0);

StructuredBuffer<Frustum> in_Frustums : register(t1);

RWStructuredBuffer<uint> g_LightIndexCounter : register(u0);
RWStructuredBuffer<uint> g_LightIndexList : register(u1);
RWTexture2D<uint2> g_LightGrid : register(u2);
RWTexture2D<float4> g_DebugTexture : register(u3);

groupshared uint l_uMinDepth;
groupshared uint l_uMaxDepth;
groupshared Frustum l_TileFrustum;
groupshared uint l_LightCount;
groupshared uint l_LightIndexStartOffset;
groupshared uint l_LightList[256];

static float4 heatArray[8] = {
	float4(0.0f, 0.0f, 0.5f, 1.0f),
	float4(0.0f, 0.0f, 1.0f, 1.0f),
	float4(0.0f, 1.0f, 0.5f, 1.0f),
	float4(0.0f, 1.0f, 1.0f, 1.0f),
	float4(0.0f, 0.5f, 0.0f, 1.0f),
	float4(0.0f, 1.0f, 0.0f, 1.0f),
	float4(1.0f, 1.0f, 0.0f, 1.0f),
	float4(1.0f, 0.0f, 0.0f, 1.0f)
};

void AppendLight(uint lightIndex)
{
	uint index; // Index into the visible lights array.
	InterlockedAdd(l_LightCount, 1, index);
	if (index < 256)
	{
		l_LightList[index] = lightIndex;
	}
}

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void main(ComputeInputType input)
{
	// Calculate min & max depth in threadgroup / tile.
	int2 texCoord = input.dispatchThreadID.xy;
	float fDepth = DepthTextureVS.Load(int3(texCoord, 0)).r;

	uint uDepth = asuint(fDepth);

	// Initialize group local variables
	// Avoid contention by other threads in the group.
	if (input.groupIndex == 0)
	{
		l_uMinDepth = 0xffffffff;
		l_uMaxDepth = 0;
		l_LightCount = 0;
		l_TileFrustum = in_Frustums[input.groupID.x + (input.groupID.y * numThreadGroups.x)];
	}

	GroupMemoryBarrierWithGroupSync();

	// Get min/max depth
	InterlockedMin(l_uMinDepth, uDepth);
	InterlockedMax(l_uMaxDepth, uDepth);

	GroupMemoryBarrierWithGroupSync();

	float fMinDepth = asfloat(l_uMinDepth);
	float fMaxDepth = asfloat(l_uMaxDepth);

	// Convert depth values to view space.
	float minDepthVS = ClipToView(float4(0, 0, fMinDepth, 1), p_inv).z;
	float maxDepthVS = ClipToView(float4(0, 0, fMaxDepth, 1), p_inv).z;
	float nearClipVS = ClipToView(float4(0, 0, 0, 1), p_inv).z;

	// Clipping plane for minimum depth value
	Plane minPlane = { float3(0, 0, -1), -minDepthVS };

	// Cull point light
	for (uint i = input.groupIndex; i < NR_POINT_LIGHTS; i += BLOCK_SIZE * BLOCK_SIZE)
	{
		pointLight light = pointLights[i];
		// w component of luminance is the attenuation radius
		if (light.luminance.w > 0.0f)
		{
			float4 lightPos_VS = mul(light.position, cam_t);
			lightPos_VS = mul(lightPos_VS, cam_r);
			Sphere sphere = { lightPos_VS.xyz, light.luminance.w };
			if (SphereInsideFrustum(sphere, l_TileFrustum, nearClipVS, maxDepthVS))
			{
				if (!SphereInsidePlane(sphere, minPlane))
				{
					AppendLight(i);
				}
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();

	// Only local thread 0 could write to the global light index counter and light grid
	if (input.groupIndex == 0)
	{
		InterlockedAdd(g_LightIndexCounter[0], l_LightCount, l_LightIndexStartOffset);
		g_LightGrid[input.groupID.xy] = uint2(l_LightIndexStartOffset, l_LightCount);
	}

	GroupMemoryBarrierWithGroupSync();

	// Write to global light index list
	for (i = input.groupIndex; i < l_LightCount; i += BLOCK_SIZE * BLOCK_SIZE)
	{
		g_LightIndexList[l_LightIndexStartOffset + i] = l_LightList[i];
	}

	// Write to debug heat map texture
	g_DebugTexture[texCoord] = float4(0, 0, 0, 0);

	if (input.groupThreadID.x == 0 || input.groupThreadID.y == 0)
	{
		g_DebugTexture[texCoord] = float4(0, 0, 0, 0.9f);
	}
	else if (input.groupThreadID.x == 1 || input.groupThreadID.y == 1)
	{
		g_DebugTexture[texCoord] = float4(1, 1, 1, 0.5f);
	}
	else if (l_LightCount > 0)
	{
		float4 heat;

		if (l_LightCount >= 8)
		{
			heat = heatArray[7];
		}
		else
		{
			heat = heatArray[l_LightCount - 1];
		}
		g_DebugTexture[texCoord] = heat;
	}
	else
	{
		g_DebugTexture[texCoord] = float4(0, 0, 0, 1);
	}
}