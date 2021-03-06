#pragma once
#include "../../Engine/Interface/IRenderPass.h"

namespace Inno
{
	class TiledFrustumGenerationPass : IRenderPass
	{
	public:
		INNO_CLASS_SINGLETON(TiledFrustumGenerationPass)

		bool Setup(ISystemConfig *systemConfig = nullptr) override;
		bool Initialize() override;
		bool Terminate() override;
		ObjectStatus GetStatus() override;

		bool PrepareCommandList(IRenderingContext* renderingContext = nullptr) override;
		RenderPassDataComponent *GetRPDC() override;

		GPUResourceComponent *GetTiledFrustum();

	private:
		ObjectStatus m_ObjectStatus;
		RenderPassDataComponent *m_RPDC;
		ShaderProgramComponent *m_SPC;
		TextureDataComponent *m_TDC;
		GPUBufferDataComponent* m_tiledFrustum;
		const uint32_t m_tileSize = 16;
		const uint32_t m_numThreadPerGroup = 16;
		InnoMath::TVec4<uint32_t> m_numThreads;
		InnoMath::TVec4<uint32_t> m_numThreadGroups;
	};
} // namespace Inno
