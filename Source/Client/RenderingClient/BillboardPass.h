#pragma once
#include "../../Engine/Interface/IRenderPass.h"

namespace Inno
{
	class BillboardPass : IRenderPass
	{
	public:
		INNO_CLASS_SINGLETON(BillboardPass)

		bool Setup(ISystemConfig *systemConfig = nullptr) override;
		bool Initialize() override;
		bool Terminate() override;
		ObjectStatus GetStatus() override;

		bool PrepareCommandList(IRenderingContext* renderingContext = nullptr) override;
		RenderPassDataComponent *GetRPDC() override;

		GPUResourceComponent *GetResult();

	private:
		ObjectStatus m_ObjectStatus;
		RenderPassDataComponent *m_RPDC;
		ShaderProgramComponent *m_SPC;
		SamplerDataComponent *m_SDC;
	};
} // namespace Inno
