#ifndef MTRenderingServerBridge_h
#define MTRenderingServerBridge_h

#include "../../Common/InnoType.h"
#include "../../Component/MTMeshDataComponent.h"
#include "../../Component/MTTextureDataComponent.h"

namespace Inno
{
	class MTRenderingServerBridge
	{
	public:
		MTRenderingServerBridge() {};
		virtual ~MTRenderingServerBridge() {};

		virtual bool Setup() = 0;
		virtual bool Initialize() = 0;
		virtual bool Update() = 0;
		virtual	bool render() = 0;
		virtual	bool present() = 0;
		virtual bool Terminate() = 0;

		virtual ObjectStatus GetStatus() = 0;

		virtual bool resize() = 0;

		virtual bool initializeMTMeshDataComponent(MTMeshDataComponent* rhs) = 0;
		virtual bool initializeMTTextureDataComponent(MTTextureDataComponent* rhs) = 0;
	};
}
#endif /* MTRenderingSystemBridge_h */
