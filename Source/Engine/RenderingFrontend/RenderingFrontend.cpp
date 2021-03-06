#include "RenderingFrontend.h"

#include "../Core/InnoLogger.h"

#include "../Interface/IEngine.h"
using namespace Inno;
extern IEngine* g_Engine;

#include "../RayTracer/RayTracer.h"

namespace InnoRenderingFrontendNS
{
	ObjectStatus m_ObjectStatus = ObjectStatus::Terminated;

	IRenderingServer* m_renderingServer;
	IRayTracer* m_rayTracer;

	TVec2<uint32_t> m_screenResolution = TVec2<uint32_t>(1280, 720);
	std::string m_windowName;
	bool m_fullScreen = false;

	RenderingCapability m_renderingCapability;

	RenderPassDesc m_DefaultRenderPassDesc;

	DoubleBuffer<PerFrameConstantBuffer, true> m_perFrameCB;

	DoubleBuffer<std::vector<CSMConstantBuffer>, true> m_CSMCBVector;

	DoubleBuffer<std::vector<PointLightConstantBuffer>, true> m_pointLightCBVector;
	DoubleBuffer<std::vector<SphereLightConstantBuffer>, true> m_sphereLightCBVector;

	uint32_t m_drawCallCount = 0;
	DoubleBuffer<std::vector<DrawCallInfo>, true> m_drawCallInfoVector;
	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_perObjectCBVector;
	DoubleBuffer<std::vector<MaterialConstantBuffer>, true> m_materialCBVector;

	DoubleBuffer<std::vector<AnimationDrawCallInfo>, true> m_animationDrawCallInfoVector;
	DoubleBuffer<std::vector<AnimationConstantBuffer>, true> m_animationCBVector;

	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_directionalLightPerObjectCB;
	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_pointLightPerObjectCB;
	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_sphereLightPerObjectCB;

	DoubleBuffer<std::vector<BillboardPassDrawCallInfo>, true> m_billboardPassDrawCallInfoVector;
	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_billboardPassPerObjectCB;

	DoubleBuffer<std::vector<DebugPassDrawCallInfo>, true> m_debugPassDrawCallInfoVector;
	DoubleBuffer<std::vector<PerObjectConstantBuffer>, true> m_debugPassPerObjectCB;

	ThreadSafeUnorderedMap<uint64_t, AnimationInstance> m_animationInstanceMap;

	std::vector<CullingData> m_cullingData;

	std::vector<Vec2> m_haltonSampler;
	int32_t m_currentHaltonStep = 0;
	int64_t m_previousTime = 0;
	int64_t m_currentTime = 0;

	std::function<void()> f_sceneLoadingStartCallback;
	std::function<void()> f_sceneLoadingFinishCallback;

	RenderingConfig m_renderingConfig = RenderingConfig();

	ThreadSafeQueue<MeshDataComponent*> m_uninitializedMeshes;
	ThreadSafeQueue<MaterialDataComponent*> m_uninitializedMaterials;
	ThreadSafeQueue<AnimationDataComponent*> m_uninitializedAnimations;

	ThreadSafeUnorderedMap<std::string, AnimationData> m_animationDataInfosLUT;

	TextureDataComponent* m_iconTemplate_DirectionalLight;
	TextureDataComponent* m_iconTemplate_PointLight;
	TextureDataComponent* m_iconTemplate_SphereLight;

	MeshDataComponent* m_unitTriangleMesh;
	MeshDataComponent* m_unitSquareMesh;
	MeshDataComponent* m_unitPentagonMesh;
	MeshDataComponent* m_unitHexagonMesh;

	MeshDataComponent* m_unitTetrahedronMesh;
	MeshDataComponent* m_unitCubeMesh;
	MeshDataComponent* m_unitOctahedronMesh;
	MeshDataComponent* m_unitDodecahedronMesh;
	MeshDataComponent* m_unitIcosahedronMesh;
	MeshDataComponent* m_unitSphereMesh;
	MeshDataComponent* m_terrainMesh;

	MaterialDataComponent* m_defaultMaterial;

	bool Setup(ISystemConfig* systemConfig);
	bool loadDefaultAssets();
	bool Initialize();
	bool Update();
	bool Terminate();

	float radicalInverse(uint32_t n, uint32_t base);
	void initializeHaltonSampler();
	void initializeAnimation(AnimationDataComponent* rhs);
	AnimationData getAnimationData(const char* animationName);

	bool updatePerFrameConstantBuffer();
	bool updateLightData();

	bool updateMeshData();
	bool simulateAnimation();
	bool updateBillboardPassData();
	bool updateDebuggerPassData();
}

using namespace InnoRenderingFrontendNS;

float InnoRenderingFrontendNS::radicalInverse(uint32_t n, uint32_t base)
{
	float val = 0.0f;
	float invBase = 1.0f / base, invBi = invBase;
	while (n > 0)
	{
		auto d_i = (n % base);
		val += d_i * invBi;
		n *= (uint32_t)invBase;
		invBi *= invBase;
	}
	return val;
};

void InnoRenderingFrontendNS::initializeHaltonSampler()
{
	// in NDC space
	for (uint32_t i = 0; i < 16; i++)
	{
		m_haltonSampler.emplace_back(Vec2(radicalInverse(i, 3) * 2.0f - 1.0f, radicalInverse(i, 4) * 2.0f - 1.0f));
	}
}

void InnoRenderingFrontendNS::initializeAnimation(AnimationDataComponent* rhs)
{
	std::string l_name = rhs->m_InstanceName.c_str();

	auto l_keyData = g_Engine->getRenderingServer()->AddGPUBufferDataComponent((l_name + "_KeyData").c_str());
	l_keyData->m_Owner = rhs->m_Owner;
	l_keyData->m_ElementCount = rhs->m_KeyData.capacity();
	l_keyData->m_ElementSize = sizeof(KeyData);
	l_keyData->m_GPUAccessibility = Accessibility::ReadWrite;

	g_Engine->getRenderingServer()->InitializeGPUBufferDataComponent(l_keyData);
	g_Engine->getRenderingServer()->UploadGPUBufferDataComponent(l_keyData, &rhs->m_KeyData[0]);

	rhs->m_ObjectStatus = ObjectStatus::Activated;

	AnimationData l_info;
	l_info.ADC = rhs;
	l_info.keyData = l_keyData;

	m_animationDataInfosLUT.emplace(rhs->m_InstanceName.c_str(), l_info);
}

AnimationData InnoRenderingFrontendNS::getAnimationData(const char* animationName)
{
	auto l_result = m_animationDataInfosLUT.find(animationName);
	if (l_result != m_animationDataInfosLUT.end())
	{
		return l_result->second;
	}
	else
	{
		return AnimationData();
	}
}

bool InnoRenderingFrontendNS::Setup(ISystemConfig* systemConfig)
{
	auto l_renderingFrontendConfig = reinterpret_cast<IRenderingFrontendConfig*>(systemConfig);

	m_renderingServer = l_renderingFrontendConfig->m_RenderingServer;
	m_rayTracer = new InnoRayTracer();

	m_renderingConfig.useCSM = true;
	m_renderingConfig.useMotionBlur = true;
	//m_renderingConfig.useTAA = true;
	//m_renderingConfig.useBloom = true;
	m_renderingConfig.drawSky = true;
	//m_renderingConfig.drawTerrain = true;
	m_renderingConfig.drawDebugObject = true;
	m_renderingConfig.CSMFitToScene = true;
	m_renderingConfig.CSMAdjustDrawDistance = true;
	m_renderingConfig.CSMAdjustSidePlane = false;

	m_renderingCapability.maxCSMSplits = 4;
	m_renderingCapability.maxPointLights = 1024;
	m_renderingCapability.maxSphereLights = 128;
	m_renderingCapability.maxMeshes = 4096;
	m_renderingCapability.maxMaterials = 4096;
	m_renderingCapability.maxTextures = 4096;

	m_DefaultRenderPassDesc.m_UseMultiFrames = false;
	m_DefaultRenderPassDesc.m_RenderTargetCount = 1;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.Sampler = TextureSampler::Sampler2D;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.Usage = TextureUsage::ColorAttachment;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.PixelDataFormat = TexturePixelDataFormat::RGBA;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.Width = m_screenResolution.x;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.Height = m_screenResolution.y;
	m_DefaultRenderPassDesc.m_RenderTargetDesc.PixelDataType = TexturePixelDataType::Float16;

	m_DefaultRenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Width = (float)m_screenResolution.x;
	m_DefaultRenderPassDesc.m_GraphicsPipelineDesc.m_ViewportDesc.m_Height = (float)m_screenResolution.y;

	m_previousTime = g_Engine->getTimeSystem()->getCurrentTimeFromEpoch();
	m_currentTime = g_Engine->getTimeSystem()->getCurrentTimeFromEpoch();

	m_CSMCBVector.Reserve(m_renderingCapability.maxCSMSplits);

	m_drawCallInfoVector.Reserve(m_renderingCapability.maxMeshes);
	m_perObjectCBVector.Reserve(m_renderingCapability.maxMeshes);
	m_materialCBVector.Reserve(m_renderingCapability.maxMaterials);
	m_animationDrawCallInfoVector.Reserve(512);
	m_animationCBVector.Reserve(512);

	m_pointLightCBVector.Reserve(m_renderingCapability.maxPointLights);
	m_sphereLightCBVector.Reserve(m_renderingCapability.maxSphereLights);

	m_directionalLightPerObjectCB.Reserve(4096);
	m_pointLightPerObjectCB.Reserve(4096);
	m_sphereLightPerObjectCB.Reserve(4096);
	m_billboardPassPerObjectCB.Reserve(4096);

	f_sceneLoadingStartCallback = [&]() {
		m_cullingData.clear();

		m_drawCallCount = 0;
	};

	f_sceneLoadingFinishCallback = [&]()
	{
		// @TODO:
		std::vector<BillboardPassDrawCallInfo> l_billboardPassDrawCallInfoVectorA(3);
		l_billboardPassDrawCallInfoVectorA[0].iconTexture = g_Engine->getRenderingFrontend()->getTextureDataComponent(WorldEditorIconType::DIRECTIONAL_LIGHT);
		l_billboardPassDrawCallInfoVectorA[1].iconTexture = g_Engine->getRenderingFrontend()->getTextureDataComponent(WorldEditorIconType::POINT_LIGHT);
		l_billboardPassDrawCallInfoVectorA[2].iconTexture = g_Engine->getRenderingFrontend()->getTextureDataComponent(WorldEditorIconType::SPHERE_LIGHT);
		auto l_billboardPassDrawCallInfoVectorB = l_billboardPassDrawCallInfoVectorA;

		m_billboardPassDrawCallInfoVector.SetValue(std::move(l_billboardPassDrawCallInfoVectorA));
		m_billboardPassDrawCallInfoVector.SetValue(std::move(l_billboardPassDrawCallInfoVectorB));
	};

	g_Engine->getSceneSystem()->addSceneLoadingStartCallback(&f_sceneLoadingStartCallback);
	g_Engine->getSceneSystem()->addSceneLoadingFinishCallback(&f_sceneLoadingFinishCallback);

	g_Engine->getComponentManager()->RegisterType<SkeletonDataComponent>(2048);
	g_Engine->getComponentManager()->RegisterType<AnimationDataComponent>(16384);

	m_rayTracer->Setup();

	m_ObjectStatus = ObjectStatus::Created;
	return true;
}

bool InnoRenderingFrontendNS::loadDefaultAssets()
{
	auto m_basicNormalTexture = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//basic_normal.png");
	m_basicNormalTexture->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_basicNormalTexture->m_TextureDesc.Usage = TextureUsage::Sample;

	auto m_basicAlbedoTexture = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//basic_albedo.png");
	m_basicAlbedoTexture->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_basicAlbedoTexture->m_TextureDesc.Usage = TextureUsage::Sample;

	auto m_basicMetallicTexture = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//basic_metallic.png");
	m_basicMetallicTexture->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_basicMetallicTexture->m_TextureDesc.Usage = TextureUsage::Sample;

	auto m_basicRoughnessTexture = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//basic_roughness.png");
	m_basicRoughnessTexture->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_basicRoughnessTexture->m_TextureDesc.Usage = TextureUsage::Sample;

	auto m_basicAOTexture = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//basic_ao.png");
	m_basicAOTexture->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_basicAOTexture->m_TextureDesc.Usage = TextureUsage::Sample;

	m_defaultMaterial = m_renderingServer->AddMaterialDataComponent("BasicMaterial/");
	m_defaultMaterial->m_TextureSlots[0].m_Texture = m_basicNormalTexture;
	m_defaultMaterial->m_TextureSlots[1].m_Texture = m_basicAlbedoTexture;
	m_defaultMaterial->m_TextureSlots[2].m_Texture = m_basicMetallicTexture;
	m_defaultMaterial->m_TextureSlots[3].m_Texture = m_basicRoughnessTexture;
	m_defaultMaterial->m_TextureSlots[4].m_Texture = m_basicAOTexture;
	m_defaultMaterial->m_TextureSlots[5].m_Texture = m_basicAOTexture;
	m_defaultMaterial->m_TextureSlots[6].m_Texture = m_basicAOTexture;
	m_defaultMaterial->m_TextureSlots[7].m_Texture = m_basicAOTexture;
	m_defaultMaterial->m_ShaderModel = ShaderModel::Opaque;

	m_iconTemplate_DirectionalLight = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//InnoWorldEditorIcons_DirectionalLight.png");
	m_iconTemplate_DirectionalLight->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_iconTemplate_DirectionalLight->m_TextureDesc.Usage = TextureUsage::Sample;

	m_iconTemplate_PointLight = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//InnoWorldEditorIcons_PointLight.png");
	m_iconTemplate_PointLight->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_iconTemplate_PointLight->m_TextureDesc.Usage = TextureUsage::Sample;

	m_iconTemplate_SphereLight = g_Engine->getAssetSystem()->loadTexture("..//Res//Textures//InnoWorldEditorIcons_SphereLight.png");
	m_iconTemplate_SphereLight->m_TextureDesc.Sampler = TextureSampler::Sampler2D;
	m_iconTemplate_SphereLight->m_TextureDesc.Usage = TextureUsage::Sample;

	m_unitTriangleMesh = m_renderingServer->AddMeshDataComponent("UnitTriangleMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Triangle, m_unitTriangleMesh);
	m_unitTriangleMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitTriangleMesh->m_proceduralMeshShape = ProceduralMeshShape::Triangle;
	m_unitTriangleMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitSquareMesh = m_renderingServer->AddMeshDataComponent("UnitSquareMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Square, m_unitSquareMesh);
	m_unitSquareMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitSquareMesh->m_proceduralMeshShape = ProceduralMeshShape::Square;
	m_unitSquareMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitPentagonMesh = m_renderingServer->AddMeshDataComponent("UnitPentagonMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Pentagon, m_unitPentagonMesh);
	m_unitPentagonMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitPentagonMesh->m_proceduralMeshShape = ProceduralMeshShape::Pentagon;
	m_unitPentagonMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitHexagonMesh = m_renderingServer->AddMeshDataComponent("UnitHexagonMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Hexagon, m_unitHexagonMesh);
	m_unitHexagonMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitHexagonMesh->m_proceduralMeshShape = ProceduralMeshShape::Hexagon;
	m_unitHexagonMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitTetrahedronMesh = m_renderingServer->AddMeshDataComponent("UnitTetrahedronMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Tetrahedron, m_unitTetrahedronMesh);
	m_unitTetrahedronMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitTetrahedronMesh->m_proceduralMeshShape = ProceduralMeshShape::Tetrahedron;
	m_unitTetrahedronMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitCubeMesh = m_renderingServer->AddMeshDataComponent("UnitCubeMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Cube, m_unitCubeMesh);
	m_unitCubeMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitCubeMesh->m_proceduralMeshShape = ProceduralMeshShape::Cube;
	m_unitCubeMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitOctahedronMesh = m_renderingServer->AddMeshDataComponent("UnitOctahedronMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Octahedron, m_unitOctahedronMesh);
	m_unitOctahedronMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitOctahedronMesh->m_proceduralMeshShape = ProceduralMeshShape::Octahedron;
	m_unitOctahedronMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitDodecahedronMesh = m_renderingServer->AddMeshDataComponent("UnitDodecahedronMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Dodecahedron, m_unitDodecahedronMesh);
	m_unitDodecahedronMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitDodecahedronMesh->m_proceduralMeshShape = ProceduralMeshShape::Dodecahedron;
	m_unitDodecahedronMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitIcosahedronMesh = m_renderingServer->AddMeshDataComponent("UnitIcosahedronMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Icosahedron, m_unitIcosahedronMesh);
	m_unitIcosahedronMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitIcosahedronMesh->m_proceduralMeshShape = ProceduralMeshShape::Icosahedron;
	m_unitIcosahedronMesh->m_ObjectStatus = ObjectStatus::Created;

	m_unitSphereMesh = m_renderingServer->AddMeshDataComponent("UnitSphereMesh/");
	g_Engine->getAssetSystem()->generateProceduralMesh(ProceduralMeshShape::Sphere, m_unitSphereMesh);
	m_unitSphereMesh->m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	m_unitSphereMesh->m_proceduralMeshShape = ProceduralMeshShape::Sphere;
	m_unitSphereMesh->m_ObjectStatus = ObjectStatus::Created;

	auto l_DefaultAssetInitializeTask = g_Engine->getTaskSystem()->Submit("DefaultAssetInitializeTask", 2, nullptr,
		[&]() {
			m_renderingServer->InitializeMeshDataComponent(m_unitTriangleMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitSquareMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitPentagonMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitHexagonMesh);

			m_renderingServer->InitializeMeshDataComponent(m_unitTetrahedronMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitCubeMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitOctahedronMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitDodecahedronMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitIcosahedronMesh);
			m_renderingServer->InitializeMeshDataComponent(m_unitSphereMesh);

			m_renderingServer->InitializeTextureDataComponent(m_basicNormalTexture);
			m_renderingServer->InitializeTextureDataComponent(m_basicAlbedoTexture);
			m_renderingServer->InitializeTextureDataComponent(m_basicMetallicTexture);
			m_renderingServer->InitializeTextureDataComponent(m_basicRoughnessTexture);
			m_renderingServer->InitializeTextureDataComponent(m_basicAOTexture);

			m_renderingServer->InitializeTextureDataComponent(m_iconTemplate_DirectionalLight);
			m_renderingServer->InitializeTextureDataComponent(m_iconTemplate_PointLight);
			m_renderingServer->InitializeTextureDataComponent(m_iconTemplate_SphereLight);

			m_renderingServer->InitializeMaterialDataComponent(m_defaultMaterial);
		});

	l_DefaultAssetInitializeTask.m_Future->Get();

	return true;
}

bool InnoRenderingFrontendNS::Initialize()
{
	if (m_ObjectStatus == ObjectStatus::Created)
	{
		loadDefaultAssets();

		initializeHaltonSampler();
		m_rayTracer->Initialize();

		m_ObjectStatus = ObjectStatus::Activated;
		InnoLogger::Log(LogLevel::Success, "RenderingFrontend has been initialized.");
		return true;
	}
	else
	{
		InnoLogger::Log(LogLevel::Error, "RenderingFrontend: Object is not created!");
		return false;
	}
}

bool InnoRenderingFrontendNS::updatePerFrameConstantBuffer()
{
	auto l_mainCamera = g_Engine->getComponentManager()->Get<CameraComponent>(0);

	if (l_mainCamera == nullptr)
	{
		return false;
	}

	auto l_mainCameraTransformComponent = g_Engine->getComponentManager()->Find<TransformComponent>(l_mainCamera->m_Owner);

	if (l_mainCameraTransformComponent == nullptr)
	{
		return false;
	}

	PerFrameConstantBuffer l_PerFrameCB;

	auto l_p = l_mainCamera->m_projectionMatrix;

	l_PerFrameCB.p_original = l_p;
	l_PerFrameCB.p_jittered = l_p;

	if (m_renderingConfig.useTAA)
	{
		//TAA jitter for projection matrix
		auto& l_currentHaltonStep = m_currentHaltonStep;
		if (l_currentHaltonStep >= 16)
		{
			l_currentHaltonStep = 0;
		}
		l_PerFrameCB.p_jittered.m02 = m_haltonSampler[l_currentHaltonStep].x / m_screenResolution.x;
		l_PerFrameCB.p_jittered.m12 = m_haltonSampler[l_currentHaltonStep].y / m_screenResolution.y;
		l_currentHaltonStep += 1;
	}

	auto r = InnoMath::getInvertRotationMatrix(l_mainCameraTransformComponent->m_globalTransformVector.m_rot);

	auto t = InnoMath::getInvertTranslationMatrix(l_mainCameraTransformComponent->m_globalTransformVector.m_pos);

	l_PerFrameCB.camera_posWS = l_mainCameraTransformComponent->m_globalTransformVector.m_pos;

	l_PerFrameCB.v = r * t;

	auto r_prev = l_mainCameraTransformComponent->m_globalTransformMatrix_prev.m_rotationMat.inverse();
	auto t_prev = l_mainCameraTransformComponent->m_globalTransformMatrix_prev.m_translationMat.inverse();

	l_PerFrameCB.v_prev = r_prev * t_prev;

	l_PerFrameCB.zNear = l_mainCamera->m_zNear;
	l_PerFrameCB.zFar = l_mainCamera->m_zFar;

	l_PerFrameCB.p_inv = l_p.inverse();
	l_PerFrameCB.v_inv = l_PerFrameCB.v.inverse();
	l_PerFrameCB.viewportSize.x = (float)m_screenResolution.x;
	l_PerFrameCB.viewportSize.y = (float)m_screenResolution.y;
	l_PerFrameCB.minLogLuminance = -10.0f;
	l_PerFrameCB.maxLogLuminance = 16.0f;
	l_PerFrameCB.aperture = l_mainCamera->m_aperture;
	l_PerFrameCB.shutterTime = l_mainCamera->m_shutterTime;
	l_PerFrameCB.ISO = l_mainCamera->m_ISO;

	auto l_sun = g_Engine->getComponentManager()->Get<LightComponent>(0);

	if (l_sun == nullptr)
	{
		return false;
	}

	auto l_sunTransformComponent = g_Engine->getComponentManager()->Find<TransformComponent>(l_sun->m_Owner);

	if (l_sunTransformComponent == nullptr)
	{
		return false;
	}

	l_PerFrameCB.sun_direction = InnoMath::getDirection(Direction::Backward, l_sunTransformComponent->m_globalTransformVector.m_rot);
	l_PerFrameCB.sun_illuminance = l_sun->m_RGBColor * l_sun->m_LuminousFlux;

	m_perFrameCB.SetValue(std::move(l_PerFrameCB));

	auto& l_SplitAABB = l_sun->m_SplitAABBWS;
	auto& l_ViewMatrices = l_sun->m_ViewMatrices;
	auto& l_ProjectionMatrices = l_sun->m_ProjectionMatrices;

	auto& l_CSMCBVector = m_CSMCBVector.GetValue();
	l_CSMCBVector.clear();

	if (l_SplitAABB.size() > 0 && l_ViewMatrices.size() > 0 && l_ProjectionMatrices.size() > 0)
	{
		for (size_t j = 0; j < l_SplitAABB.size(); j++)
		{
			CSMConstantBuffer l_CSMCB;

			l_CSMCB.p = l_ProjectionMatrices[j];
			l_CSMCB.v = l_ViewMatrices[j];

			l_CSMCB.AABBMax = l_SplitAABB[j].m_boundMax;
			l_CSMCB.AABBMin = l_SplitAABB[j].m_boundMin;

			l_CSMCBVector.emplace_back(l_CSMCB);
		}
	}

	return true;
}

bool InnoRenderingFrontendNS::updateLightData()
{
	auto& l_PointLightCB = m_pointLightCBVector.GetValue();
	auto& l_SphereLightCB = m_sphereLightCBVector.GetValue();

	l_PointLightCB.clear();
	l_SphereLightCB.clear();

	auto& l_lightComponents = g_Engine->getComponentManager()->GetAll<LightComponent>();
	auto l_lightComponentCount = l_lightComponents.size();

	if (l_lightComponentCount > 0)
	{
		for (size_t i = 0; i < l_lightComponentCount; i++)
		{
			auto l_transformCompoent = g_Engine->getComponentManager()->Find<TransformComponent>(l_lightComponents[i]->m_Owner);
			if (l_transformCompoent != nullptr)
			{
				if (l_lightComponents[i]->m_LightType == LightType::Point)
				{
					PointLightConstantBuffer l_data;
					l_data.pos = l_transformCompoent->m_globalTransformVector.m_pos;
					l_data.luminance = l_lightComponents[i]->m_RGBColor * l_lightComponents[i]->m_LuminousFlux;
					l_data.luminance.w = l_lightComponents[i]->m_Shape.x;
					l_PointLightCB.emplace_back(l_data);
				}
				else if (l_lightComponents[i]->m_LightType == LightType::Sphere)
				{
					SphereLightConstantBuffer l_data;
					l_data.pos = l_transformCompoent->m_globalTransformVector.m_pos;
					l_data.luminance = l_lightComponents[i]->m_RGBColor * l_lightComponents[i]->m_LuminousFlux;
					l_data.luminance.w = l_lightComponents[i]->m_Shape.x;
					l_SphereLightCB.emplace_back(l_data);
				}
			}
		}
	}

	return true;
}

bool InnoRenderingFrontendNS::updateMeshData()
{
	auto& l_drawCallInfoVector = m_drawCallInfoVector.GetValue();
	auto& l_perObjectCBVector = m_perObjectCBVector.GetValue();
	auto& l_materialCBVector = m_materialCBVector.GetValue();
	auto& l_animationDrawCallInfoVector = m_animationDrawCallInfoVector.GetValue();
	auto& l_animationCBVector = m_animationCBVector.GetValue();

	l_drawCallInfoVector.clear();
	l_perObjectCBVector.clear();
	l_materialCBVector.clear();
	l_animationDrawCallInfoVector.clear();
	l_animationCBVector.clear();

	auto l_cullingDataSize = m_cullingData.size();

	for (size_t i = 0; i < l_cullingDataSize; i++)
	{
		auto l_cullingData = m_cullingData[i];
		if (l_cullingData.mesh != nullptr)
		{
			if (l_cullingData.mesh->m_ObjectStatus == ObjectStatus::Activated)
			{
				if (l_cullingData.material != nullptr)
				{
					DrawCallInfo l_drawCallInfo;

					l_drawCallInfo.mesh = l_cullingData.mesh;
					l_drawCallInfo.material = l_cullingData.material;

					l_drawCallInfo.visibilityMask = l_cullingData.visibilityMask;
					l_drawCallInfo.meshUsage = l_cullingData.meshUsage;
					l_drawCallInfo.meshConstantBufferIndex = (uint32_t)i;
					l_drawCallInfo.materialConstantBufferIndex = (uint32_t)i;

					PerObjectConstantBuffer l_perObjectCB;
					l_perObjectCB.m = l_cullingData.m;
					l_perObjectCB.m_prev = l_cullingData.m_prev;
					l_perObjectCB.normalMat = l_cullingData.normalMat;
					l_perObjectCB.UUID = (float)l_cullingData.UUID;

					MaterialConstantBuffer l_materialCB;

					for (size_t i = 0; i < 8; i++)
					{
						uint32_t l_writeMask = l_drawCallInfo.material->m_TextureSlots[i].m_Activate ? 0x00000001 : 0x00000000;
						l_writeMask = l_writeMask << i;
						l_materialCB.textureSlotMask |= l_writeMask;
					}
					l_materialCB.materialType = int32_t(l_cullingData.meshUsage);
					l_materialCB.materialAttributes = l_cullingData.material->m_materialAttributes;

					if (l_cullingData.meshUsage == MeshUsage::Skeletal)
					{
						auto l_result = m_animationInstanceMap.find(l_cullingData.UUID);
						if (l_result != m_animationInstanceMap.end())
						{
							AnimationDrawCallInfo animationDrawCallInfo;
							animationDrawCallInfo.animationInstance = l_result->second;
							animationDrawCallInfo.drawCallInfo = l_drawCallInfo;

							AnimationConstantBuffer l_animationCB;
							l_animationCB.duration = animationDrawCallInfo.animationInstance.animationData.ADC->m_Duration;
							l_animationCB.numChannels = animationDrawCallInfo.animationInstance.animationData.ADC->m_NumChannels;
							l_animationCB.numTicks = animationDrawCallInfo.animationInstance.animationData.ADC->m_NumTicks;
							l_animationCB.currentTime = animationDrawCallInfo.animationInstance.currentTime / l_animationCB.duration;
							l_animationCB.rootOffsetMatrix = InnoMath::generateIdentityMatrix<float>();

							l_animationCBVector.emplace_back(l_animationCB);

							animationDrawCallInfo.animationConstantBufferIndex = (uint32_t)l_animationCBVector.size();
							l_animationDrawCallInfoVector.emplace_back(animationDrawCallInfo);
						}
					}
					else
					{
						l_drawCallInfoVector.emplace_back(l_drawCallInfo);
					}
					l_perObjectCBVector.emplace_back(l_perObjectCB);
					l_materialCBVector.emplace_back(l_materialCB);
				}
			}
		}
	}

	// @TODO: use GPU to do OIT

	return true;
}

bool InnoRenderingFrontendNS::simulateAnimation()
{
	m_currentTime = g_Engine->getTimeSystem()->getCurrentTimeFromEpoch();

	float l_tickTime = float(m_currentTime - m_previousTime) / 1000.0f;

	if (m_animationInstanceMap.size())
	{
		for (auto& i : m_animationInstanceMap)
		{
			if (!i.second.isFinished)
			{
				if (i.second.currentTime < i.second.animationData.ADC->m_Duration)
				{
					i.second.currentTime += l_tickTime / 60.0f;
				}
				else
				{
					if (i.second.isLooping)
					{
						i.second.currentTime -= i.second.animationData.ADC->m_Duration;
					}
					else
					{
						i.second.isFinished = true;
					}
				}
			}
		}

		m_animationInstanceMap.erase_if([](auto it) { return it.second.isFinished; });
	}

	m_previousTime = m_currentTime;

	return true;
}

bool InnoRenderingFrontendNS::updateBillboardPassData()
{
	auto& l_lightComponents = g_Engine->getComponentManager()->GetAll<LightComponent>();

	auto l_totalBillboardDrawCallCount = l_lightComponents.size();

	if (l_totalBillboardDrawCallCount == 0)
	{
		return false;
	}

	auto& l_billboardPassDrawCallInfoVector = m_billboardPassDrawCallInfoVector.GetValue();
	auto& l_billboardPassPerObjectCB = m_billboardPassPerObjectCB.GetValue();
	auto& l_directionalLightPerObjectCB = m_directionalLightPerObjectCB.GetValue();
	auto& l_pointLightPerObjectCB = m_pointLightPerObjectCB.GetValue();
	auto& l_sphereLightPerObjectCB = m_sphereLightPerObjectCB.GetValue();

	auto l_billboardPassDrawCallInfoCount = l_billboardPassDrawCallInfoVector.size();
	for (size_t i = 0; i < l_billboardPassDrawCallInfoCount; i++)
	{
		l_billboardPassDrawCallInfoVector[i].instanceCount = 0;
	}

	l_billboardPassPerObjectCB.clear();

	l_directionalLightPerObjectCB.clear();
	l_pointLightPerObjectCB.clear();
	l_sphereLightPerObjectCB.clear();

	for (auto i : l_lightComponents)
	{
		PerObjectConstantBuffer l_meshCB;

		auto l_transformCompoent = g_Engine->getComponentManager()->Find<TransformComponent>(i->m_Owner);
		if (l_transformCompoent != nullptr)
		{
			l_meshCB.m = InnoMath::toTranslationMatrix(l_transformCompoent->m_globalTransformVector.m_pos);
		}

		switch (i->m_LightType)
		{
		case LightType::Directional:
			l_directionalLightPerObjectCB.emplace_back(l_meshCB);
			l_billboardPassDrawCallInfoVector[0].instanceCount++;
			break;
		case LightType::Point:
			l_pointLightPerObjectCB.emplace_back(l_meshCB);
			l_billboardPassDrawCallInfoVector[1].instanceCount++;
			break;
		case LightType::Spot:
			break;
		case LightType::Sphere:
			l_sphereLightPerObjectCB.emplace_back(l_meshCB);
			l_billboardPassDrawCallInfoVector[2].instanceCount++;
			break;
		case LightType::Disk:
			break;
		case LightType::Tube:
			break;
		case LightType::Rectangle:
			break;
		default:
			break;
		}
	}

	l_billboardPassDrawCallInfoVector[0].meshConstantBufferOffset = 0;
	l_billboardPassDrawCallInfoVector[1].meshConstantBufferOffset = (uint32_t)l_directionalLightPerObjectCB.size();
	l_billboardPassDrawCallInfoVector[2].meshConstantBufferOffset = (uint32_t)(l_directionalLightPerObjectCB.size() + l_pointLightPerObjectCB.size());

	l_billboardPassPerObjectCB.insert(l_billboardPassPerObjectCB.end(), l_directionalLightPerObjectCB.begin(), l_directionalLightPerObjectCB.end());
	l_billboardPassPerObjectCB.insert(l_billboardPassPerObjectCB.end(), l_pointLightPerObjectCB.begin(), l_pointLightPerObjectCB.end());
	l_billboardPassPerObjectCB.insert(l_billboardPassPerObjectCB.end(), l_sphereLightPerObjectCB.begin(), l_sphereLightPerObjectCB.end());

	return true;
}

bool InnoRenderingFrontendNS::updateDebuggerPassData()
{
	// @TODO: Implementation

	return true;
}

bool InnoRenderingFrontendNS::Update()
{
	if (m_ObjectStatus == ObjectStatus::Activated)
	{
		simulateAnimation();

		updatePerFrameConstantBuffer();

		updateLightData();

		// copy culling data pack for local scope
		m_cullingData = g_Engine->getPhysicsSystem()->getCullingData();

		updateMeshData();

		updateBillboardPassData();

		updateDebuggerPassData();

		return true;
	}
	else
	{
		m_ObjectStatus = ObjectStatus::Suspended;
		return false;
	}
}

bool InnoRenderingFrontendNS::Terminate()
{
	m_rayTracer->Terminate();

	m_ObjectStatus = ObjectStatus::Terminated;
	InnoLogger::Log(LogLevel::Success, "RenderingFrontend has been terminated.");
	return true;
}

bool InnoRenderingFrontend::Setup(ISystemConfig* systemConfig)
{
	return InnoRenderingFrontendNS::Setup(systemConfig);
}

bool InnoRenderingFrontend::Initialize()
{
	return InnoRenderingFrontendNS::Initialize();
}

bool InnoRenderingFrontend::Update()
{
	return InnoRenderingFrontendNS::Update();
}

bool InnoRenderingFrontend::Terminate()
{
	return InnoRenderingFrontendNS::Terminate();
}

ObjectStatus InnoRenderingFrontend::GetStatus()
{
	return InnoRenderingFrontendNS::m_ObjectStatus;
}

bool InnoRenderingFrontend::runRayTrace()
{
	return InnoRenderingFrontendNS::m_rayTracer->Execute();
}

MeshDataComponent* InnoRenderingFrontend::addMeshDataComponent()
{
	return InnoRenderingFrontendNS::m_renderingServer->AddMeshDataComponent();
}

TextureDataComponent* InnoRenderingFrontend::addTextureDataComponent()
{
	return InnoRenderingFrontendNS::m_renderingServer->AddTextureDataComponent();
}

MaterialDataComponent* InnoRenderingFrontend::addMaterialDataComponent()
{
	return InnoRenderingFrontendNS::m_renderingServer->AddMaterialDataComponent();
}

SkeletonDataComponent* InnoRenderingFrontend::addSkeletonDataComponent()
{
	static std::atomic<uint32_t> skeletonCount = 0;
	auto l_parentEntity = g_Engine->getEntityManager()->Spawn(false, ObjectLifespan::Persistence, ("Skeleton_" + std::to_string(skeletonCount) + "/").c_str());
	auto l_SDC = g_Engine->getComponentManager()->Spawn<SkeletonDataComponent>(l_parentEntity, false, ObjectLifespan::Persistence);
	l_SDC->m_Owner = l_parentEntity;
	l_SDC->m_Serializable = false;
	l_SDC->m_ObjectStatus = ObjectStatus::Created;
	l_SDC->m_ObjectLifespan = ObjectLifespan::Persistence;
	skeletonCount++;
	return l_SDC;
}

AnimationDataComponent* InnoRenderingFrontend::addAnimationDataComponent()
{
	static std::atomic<uint32_t> animationCount = 0;
	auto l_parentEntity = g_Engine->getEntityManager()->Spawn(false, ObjectLifespan::Persistence, ("Animation_" + std::to_string(animationCount) + "/").c_str());
	auto l_ADC = g_Engine->getComponentManager()->Spawn<AnimationDataComponent>(l_parentEntity, false, ObjectLifespan::Persistence);
	l_ADC->m_Owner = l_parentEntity;
	l_ADC->m_Serializable = false;
	l_ADC->m_ObjectStatus = ObjectStatus::Created;
	l_ADC->m_ObjectLifespan = ObjectLifespan::Persistence;
	animationCount++;
	return l_ADC;
}

MeshDataComponent* InnoRenderingFrontend::getMeshDataComponent(ProceduralMeshShape shape)
{
	switch (shape)
	{
	case InnoType::ProceduralMeshShape::Triangle:
		return m_unitTriangleMesh;
		break;
	case InnoType::ProceduralMeshShape::Square:
		return m_unitSquareMesh;
		break;
	case InnoType::ProceduralMeshShape::Pentagon:
		return m_unitPentagonMesh;
		break;
	case InnoType::ProceduralMeshShape::Hexagon:
		return m_unitHexagonMesh;
		break;
	case InnoType::ProceduralMeshShape::Tetrahedron:
		return m_unitTetrahedronMesh;
		break;
	case InnoType::ProceduralMeshShape::Cube:
		return m_unitCubeMesh;
		break;
	case InnoType::ProceduralMeshShape::Octahedron:
		return m_unitOctahedronMesh;
		break;
	case InnoType::ProceduralMeshShape::Dodecahedron:
		return m_unitDodecahedronMesh;
		break;
	case InnoType::ProceduralMeshShape::Icosahedron:
		return m_unitIcosahedronMesh;
		break;
	case InnoType::ProceduralMeshShape::Sphere:
		return m_unitSphereMesh;
		break;
	default:
		InnoLogger::Log(LogLevel::Error, "RenderingFrontend: Invalid ProceduralMeshShape!");
		return nullptr;
		break;
	}
}

TextureDataComponent* InnoRenderingFrontend::getTextureDataComponent(WorldEditorIconType iconType)
{
	switch (iconType)
	{
	case WorldEditorIconType::DIRECTIONAL_LIGHT:
		return m_iconTemplate_DirectionalLight; break;
	case WorldEditorIconType::POINT_LIGHT:
		return m_iconTemplate_PointLight; break;
	case WorldEditorIconType::SPHERE_LIGHT:
		return m_iconTemplate_SphereLight; break;
	default:
		return nullptr; break;
	}
}

MaterialDataComponent* InnoRenderingFrontend::getDefaultMaterialDataComponent()
{
	return m_defaultMaterial;
}

bool InnoRenderingFrontend::transferDataToGPU()
{
	while (m_uninitializedMeshes.size() > 0)
	{
		MeshDataComponent* l_Mesh;
		m_uninitializedMeshes.tryPop(l_Mesh);

		if (l_Mesh)
		{
			auto l_result = m_renderingServer->InitializeMeshDataComponent(l_Mesh);
		}
	}

	while (m_uninitializedMaterials.size() > 0)
	{
		MaterialDataComponent* l_Material;
		m_uninitializedMaterials.tryPop(l_Material);

		if (l_Material)
		{
			auto l_result = m_renderingServer->InitializeMaterialDataComponent(l_Material);
		}
	}

	while (m_uninitializedAnimations.size() > 0)
	{
		AnimationDataComponent* l_Animations;
		m_uninitializedAnimations.tryPop(l_Animations);

		if (l_Animations)
		{
			initializeAnimation(l_Animations);
		}
	}

	return true;
}

bool InnoRenderingFrontend::registerMeshDataComponent(MeshDataComponent* rhs, bool AsyncUploadToGPU)
{
	if (AsyncUploadToGPU)
	{
		m_uninitializedMeshes.push(rhs);
	}
	else
	{
		auto l_MeshDataComponentInitializeTask = g_Engine->getTaskSystem()->Submit("MeshDataComponentInitializeTask", 2, nullptr,
			[=]() { m_renderingServer->InitializeMeshDataComponent(rhs); });
		l_MeshDataComponentInitializeTask.m_Future->Get();
	}

	return true;
}

bool InnoRenderingFrontend::registerMaterialDataComponent(MaterialDataComponent* rhs, bool AsyncUploadToGPU)
{
	if (AsyncUploadToGPU)
	{
		m_uninitializedMaterials.push(rhs);
	}
	else
	{
		auto l_MaterialDataComponentInitializeTask = g_Engine->getTaskSystem()->Submit("MaterialDataComponentInitializeTask", 2, nullptr,
			[=]() { m_renderingServer->InitializeMaterialDataComponent(rhs); });
		l_MaterialDataComponentInitializeTask.m_Future->Get();
	}

	return true;
}

bool InnoRenderingFrontend::registerSkeletonDataComponent(SkeletonDataComponent* rhs, bool AsyncUploadToGPU)
{
	rhs->m_ObjectStatus = ObjectStatus::Activated;

	return true;
}

bool InnoRenderingFrontend::registerAnimationDataComponent(AnimationDataComponent* rhs, bool AsyncUploadToGPU)
{
	if (AsyncUploadToGPU)
	{
		m_uninitializedAnimations.push(rhs);
	}
	else
	{
		auto l_AnimationDataComponentInitializeTask = g_Engine->getTaskSystem()->Submit("AnimationDataComponentInitializeTask", 2, nullptr,
			[=]() { initializeAnimation(rhs); });
		l_AnimationDataComponentInitializeTask.m_Future->Get();
	}

	return true;
}

TVec2<uint32_t> InnoRenderingFrontend::getScreenResolution()
{
	return m_screenResolution;
}

bool InnoRenderingFrontend::setScreenResolution(TVec2<uint32_t> screenResolution)
{
	m_screenResolution = screenResolution;
	return true;
}

RenderingConfig InnoRenderingFrontend::getRenderingConfig()
{
	return m_renderingConfig;
}

bool InnoRenderingFrontend::setRenderingConfig(RenderingConfig renderingConfig)
{
	m_renderingConfig = renderingConfig;
	return true;
}

RenderingCapability InnoRenderingFrontend::getRenderingCapability()
{
	return m_renderingCapability;
}

RenderPassDesc InnoRenderingFrontend::getDefaultRenderPassDesc()
{
	return m_DefaultRenderPassDesc;
}

bool InnoRenderingFrontend::playAnimation(VisibleComponent* rhs, const char* animationName, bool isLooping)
{
	auto l_animationData = getAnimationData(animationName);

	if (l_animationData.ADC != nullptr)
	{
		AnimationInstance l_instance;

		l_instance.animationData = l_animationData;
		l_instance.currentTime = 0.0f;
		l_instance.isLooping = isLooping;
		l_instance.isFinished = false;

		m_animationInstanceMap.emplace(rhs->m_UUID, l_instance);

		return true;
	}

	return false;
}

bool InnoRenderingFrontend::stopAnimation(VisibleComponent* rhs, const char* animationName)
{
	auto l_result = m_animationInstanceMap.find(rhs->m_UUID);
	if (l_result != m_animationInstanceMap.end())
	{
		m_animationInstanceMap.erase(l_result->first);

		return true;
	}

	return false;
}

const PerFrameConstantBuffer& InnoRenderingFrontend::getPerFrameConstantBuffer()
{
	return m_perFrameCB.GetValue();
}

const std::vector<CSMConstantBuffer>& InnoRenderingFrontend::getCSMConstantBuffer()
{
	return m_CSMCBVector.GetValue();
}

const std::vector<PointLightConstantBuffer>& InnoRenderingFrontend::getPointLightConstantBuffer()
{
	return m_pointLightCBVector.GetValue();
}

const std::vector<SphereLightConstantBuffer>& InnoRenderingFrontend::getSphereLightConstantBuffer()
{
	return m_sphereLightCBVector.GetValue();
}

const std::vector<DrawCallInfo>& InnoRenderingFrontend::getDrawCallInfo()
{
	return m_drawCallInfoVector.GetValue();
}

const std::vector<PerObjectConstantBuffer>& InnoRenderingFrontend::getPerObjectConstantBuffer()
{
	return m_perObjectCBVector.GetValue();
}

const std::vector<MaterialConstantBuffer>& InnoRenderingFrontend::getMaterialConstantBuffer()
{
	return m_materialCBVector.GetValue();
}

const std::vector<AnimationDrawCallInfo>& InnoRenderingFrontend::getAnimationDrawCallInfo()
{
	return m_animationDrawCallInfoVector.GetValue();
}

const std::vector<AnimationConstantBuffer>& InnoRenderingFrontend::getAnimationConstantBuffer()
{
	return m_animationCBVector.GetValue();
}

const std::vector<BillboardPassDrawCallInfo>& InnoRenderingFrontend::getBillboardPassDrawCallInfo()
{
	return m_billboardPassDrawCallInfoVector.GetValue();
}

const std::vector<PerObjectConstantBuffer>& InnoRenderingFrontend::getBillboardPassPerObjectConstantBuffer()
{
	return m_billboardPassPerObjectCB.GetValue();
}

const std::vector<DebugPassDrawCallInfo>& InnoRenderingFrontend::getDebugPassDrawCallInfo()
{
	return m_debugPassDrawCallInfoVector.GetValue();
}

const std::vector<PerObjectConstantBuffer>& InnoRenderingFrontend::getDebugPassPerObjectConstantBuffer()
{
	return m_debugPassPerObjectCB.GetValue();
}