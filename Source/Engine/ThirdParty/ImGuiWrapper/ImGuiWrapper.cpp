#include "ImGuiWrapper.h"
#include "../ImGui/imgui.h"

#if defined INNO_PLATFORM_WIN
#include "ImGuiWindowWin.h"
#endif

#if defined INNO_PLATFORM_MAC
#include "ImGuiWindowMac.h"
#endif

#if defined INNO_PLATFORM_LINUX
#include "ImGuiWindowLinux.h"
#endif

#if defined INNO_RENDERER_DIRECTX
#include "ImGuiRendererDX11.h"
#endif

#if defined INNO_RENDERER_OPENGL
#include "ImGuiRendererGL.h"
#endif

#if defined INNO_RENDERER_VULKAN
#include "ImGuiRendererVK.h"
#endif

#if defined INNO_RENDERER_METAL
#include "ImGuiRendererMT.h"
#endif

#include "../../Interface/IModuleManager.h"

extern IModuleManager* g_pModuleManager;

namespace ImGuiWrapperNS
{
	void showApplicationProfiler();
	void zoom(bool zoom, ImTextureID textureID, ImVec2 renderTargetSize);

	void showWorldExplorer();
	void showTransformComponentPropertyEditor(void* rhs);
	void showVisiableComponentPropertyEditor(void* rhs);
	void showLightComponentPropertyEditor(void* rhs);
	void showConcurrencyProfiler();

	bool m_isParity = true;

	static RenderingConfig m_renderingConfig;
	static bool m_useZoom = false;
	static bool m_showRenderPassResult = false;
	static bool m_showConcurrencyProfiler = false;
	std::vector<RingBuffer<InnoTaskReport, true>> m_taskReports;

	IImGuiWindow* m_windowImpl;
	IImGuiRenderer* m_rendererImpl;
}

using namespace ImGuiWrapperNS;

bool ImGuiWrapper::setup()
{
	auto l_initConfig = g_pModuleManager->getInitConfig();

#if defined INNO_PLATFORM_WIN
	m_windowImpl = new ImGuiWindowWin();
#endif
#if defined INNO_PLATFORM_MAC
	ImGuiWrapperNS::m_isParity = false;
#endif
#if defined INNO_PLATFORM_LINUX
	ImGuiWrapperNS::m_isParity = false;
#endif

	switch (l_initConfig.renderingServer)
	{
	case RenderingServer::GL:
#if defined INNO_RENDERER_OPENGL
		m_rendererImpl = new ImGuiRendererGL();
#endif
		break;
	case RenderingServer::DX11:
#if defined INNO_RENDERER_DIRECTX
		m_rendererImpl = new ImGuiRendererDX11();
#endif
		break;
	case RenderingServer::DX12:
#if defined INNO_RENDERER_DIRECTX
		ImGuiWrapperNS::m_isParity = false;
#endif
		break;
	case RenderingServer::VK:
#if defined INNO_RENDERER_VULKAN
		m_rendererImpl = new ImGuiRendererVK();
#endif
		break;
	case RenderingServer::MT:
#if defined INNO_RENDERER_METAL
		ImGuiWrapperNS::m_isParity = false;
#endif
		break;
	default:
		break;
	}

	if (ImGuiWrapperNS::m_isParity)
	{
		// Setup Dear ImGui binding
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiWrapperNS::m_windowImpl->setup();
		ImGuiWrapperNS::m_rendererImpl->setup();
	}

	auto l_maxThreads = g_pModuleManager->getTaskSystem()->GetTotalThreadsNumber();
	m_taskReports.resize(l_maxThreads);

	return true;
}

bool ImGuiWrapper::initialize()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->initialize();
		ImGuiWrapperNS::m_rendererImpl->initialize();

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		// Setup style
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.36f, 0.36f, 0.36f, 0.50f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.71f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.75f, 0.75f, 0.75f, 0.79f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.75f, 0.76f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.36f, 0.36f, 0.36f, 0.50f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.36f, 0.36f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

		// Load Fonts
		auto l_workingDir = g_pModuleManager->getFileSystem()->getWorkingDirectory();
		l_workingDir += "..//Res//Fonts//FreeSans.otf";
		io.Fonts->AddFontFromFileTTF(l_workingDir.c_str(), 16.0f);

		ImGuiWrapperNS::m_renderingConfig = g_pModuleManager->getRenderingFrontend()->getRenderingConfig();
	}

	return true;
}

bool ImGuiWrapper::update()
{
	return true;
}

bool ImGuiWrapper::render()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_rendererImpl->newFrame();
		ImGuiWrapperNS::m_windowImpl->newFrame();

		ImGui::NewFrame();
		{
			ImGuiWrapperNS::showApplicationProfiler();
			//ImGuiWrapperNS::showFileExplorer();
			ImGuiWrapperNS::showWorldExplorer();
			ImGuiWrapperNS::showConcurrencyProfiler();
		}
		ImGui::Render();

		ImGuiWrapperNS::m_rendererImpl->render();
	}
	return true;
}

bool ImGuiWrapper::terminate()
{
	if (ImGuiWrapperNS::m_isParity)
	{
		ImGuiWrapperNS::m_windowImpl->terminate();
		ImGuiWrapperNS::m_rendererImpl->terminate();
		ImGui::DestroyContext();
	}
	return true;
}

void ImGuiWrapperNS::showApplicationProfiler()
{
	ImGui::Begin("Profiler", 0, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Checkbox("Show concurrency profiler", &m_showConcurrencyProfiler);
	ImGui::Checkbox("Use Motion Blur", &m_renderingConfig.useMotionBlur);
	ImGui::Checkbox("Use TAA", &m_renderingConfig.useTAA);
	ImGui::Checkbox("Use Bloom", &m_renderingConfig.useBloom);
	ImGui::Checkbox("Draw terrain", &m_renderingConfig.drawTerrain);
	ImGui::Checkbox("Draw sky", &m_renderingConfig.drawSky);
	ImGui::Checkbox("Draw debug object", &m_renderingConfig.drawDebugObject);
	ImGui::Checkbox("CSM fit to scene", &m_renderingConfig.CSMFitToScene);
	ImGui::Checkbox("CSM adjust draw distance", &m_renderingConfig.CSMAdjustDrawDistance);
	ImGui::Checkbox("CSM adjust side plane", &m_renderingConfig.CSMAdjustSidePlane);

	ImGui::Checkbox("Use zoom", &m_useZoom);

	const char* items[] = { "Shadow", "GI", "Opaque", "Light", "Transparent", "Terrain", "Post-processing", "Development" };

	static int32_t l_showRenderPassResultItem = 0;

	ImGui::Combo("Choose render pass", &l_showRenderPassResultItem, items, IM_ARRAYSIZE(items));

	ImGui::Checkbox("Show render pass result", &m_showRenderPassResult);

	if (m_showRenderPassResult)
	{
		ImGuiWrapperNS::m_rendererImpl->showRenderResult(RenderPassType(l_showRenderPassResultItem));
	}

	if (ImGui::Button("Run ray trace"))
	{
		g_pModuleManager->getRenderingFrontend()->runRayTrace();
	}

	static char scene_filePath[128];
	ImGui::InputText("Scene file path", scene_filePath, IM_ARRAYSIZE(scene_filePath));

	if (ImGui::Button("Save scene"))
	{
		g_pModuleManager->getFileSystem()->saveScene(scene_filePath);
	}
	if (ImGui::Button("Load scene"))
	{
		g_pModuleManager->getFileSystem()->loadScene(scene_filePath);
	}

	ImGui::End();

	g_pModuleManager->getRenderingFrontend()->setRenderingConfig(m_renderingConfig);
}

void ImGuiWrapperNS::zoom(bool zoom, ImTextureID textureID, ImVec2 renderTargetSize)
{
	if (zoom)
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImVec2 pos = ImGui::GetCursorScreenPos();
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			float region_sz = 32.0f;
			float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > renderTargetSize.x - region_sz) region_x = renderTargetSize.x - region_sz;
			float region_y = pos.y - io.MousePos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > renderTargetSize.y - region_sz) region_y = renderTargetSize.y - region_sz;
			float zoom = 4.0f;
			ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			ImVec2 uv0 = ImVec2((region_x) / renderTargetSize.x, (region_y + region_sz) / renderTargetSize.y);
			ImVec2 uv1 = ImVec2((region_x + region_sz) / renderTargetSize.x, (region_y) / renderTargetSize.y);
			ImGui::Image(textureID, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			ImGui::EndTooltip();
		}
	}
}

void ImGuiWrapperNS::showWorldExplorer()
{
	static void* selectedComponent = nullptr;
	static uint32_t selectedComponentType;

	ImGui::Begin("World Explorer", 0);
	{
		auto l_sceneHierarchyMap = g_pModuleManager->getSceneHierarchyManager()->GetSceneHierarchyMap();

		for (auto& i : l_sceneHierarchyMap)
		{
			if (i.first->m_ObjectSource == ObjectSource::Asset)
			{
				if (ImGui::TreeNode(i.first->m_Name.c_str()))
				{
					for (auto& j : i.second)
					{
						if (ImGui::Selectable(j->m_Name.c_str(), selectedComponent == j))
						{
							selectedComponent = j;
							selectedComponentType = j->m_ComponentType;
						}
					}
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Properties", 0);
	{
		if (selectedComponent)
		{
			if (selectedComponentType == 1)
			{
				showTransformComponentPropertyEditor(selectedComponent);
			}
			else if (selectedComponentType == 2)
			{
				showVisiableComponentPropertyEditor(selectedComponent);
			}
			else if (selectedComponentType == 3)
			{
				showLightComponentPropertyEditor(selectedComponent);
			}
		}
	}
	ImGui::End();
}

void ImGuiWrapperNS::showTransformComponentPropertyEditor(void* rhs)
{
	auto l_rhs = reinterpret_cast<TransformComponent*>(rhs);

	ImGui::Text("Transform Hierarchy Level: %.i", l_rhs->m_transformHierarchyLevel);

	ImGui::Text("Local Transform Vector");

	static float float_min = std::numeric_limits<float>::min();
	static float float_max = std::numeric_limits<float>::max();

	static float pos[4];
	pos[0] = l_rhs->m_localTransformVector.m_pos.x;
	pos[1] = l_rhs->m_localTransformVector.m_pos.y;
	pos[2] = l_rhs->m_localTransformVector.m_pos.z;
	pos[3] = 0.0f;

	if (ImGui::DragFloat3("Position", pos, 0.0001f, float_min, float_max))
	{
		l_rhs->m_localTransformVector_target.m_pos.x = pos[0];
		l_rhs->m_localTransformVector_target.m_pos.y = pos[1];
		l_rhs->m_localTransformVector_target.m_pos.z = pos[2];
	}

	static float rot_min = -180.0f;
	static float rot_max = 180.0f;

	static float rot[4];
	Vec4 eulerAngles = InnoMath::quatToEulerAngle(l_rhs->m_localTransformVector.m_rot);
	rot[0] = InnoMath::radianToAngle(eulerAngles.x);
	rot[1] = InnoMath::radianToAngle(eulerAngles.y);
	rot[2] = InnoMath::radianToAngle(eulerAngles.z);
	rot[3] = 0.0f;

	if (ImGui::DragFloat3("Rotation", rot, 1.0f, rot_min, rot_max))
	{
		auto roll = InnoMath::angleToRadian(rot[0]);
		auto pitch = InnoMath::angleToRadian(rot[1]);
		auto yaw = InnoMath::angleToRadian(rot[2]);

		l_rhs->m_localTransformVector_target.m_rot = InnoMath::eulerAngleToQuat(roll, pitch, yaw);
	}

	static float scale[4];
	scale[0] = l_rhs->m_localTransformVector.m_scale.x;
	scale[1] = l_rhs->m_localTransformVector.m_scale.y;
	scale[2] = l_rhs->m_localTransformVector.m_scale.z;
	scale[3] = 0.0f;

	if (ImGui::DragFloat3("Scale", scale, 0.0001f, float_min, float_max))
	{
		l_rhs->m_localTransformVector_target.m_scale.x = scale[0];
		l_rhs->m_localTransformVector_target.m_scale.y = scale[1];
		l_rhs->m_localTransformVector_target.m_scale.z = scale[2];
	}

	l_rhs->m_localTransformVector = l_rhs->m_localTransformVector_target;
}

void ImGuiWrapperNS::showVisiableComponentPropertyEditor(void* rhs)
{
	auto l_rhs = reinterpret_cast<VisibleComponent*>(rhs);

	const char* meshPrimitiveTopology_items[] = { "Point", "Line", "Triangle", "Triangle-strip" };
	static int32_t meshPrimitiveTopology_item_current = (int32_t)l_rhs->m_meshPrimitiveTopology;
	if (ImGui::Combo("Mesh primitive topology", &meshPrimitiveTopology_item_current, meshPrimitiveTopology_items, IM_ARRAYSIZE(meshPrimitiveTopology_items)))
	{
		l_rhs->m_meshPrimitiveTopology = MeshPrimitiveTopology(meshPrimitiveTopology_item_current);
	}

	static char modelFileName[128];
	ImGui::InputText("Model file name", modelFileName, IM_ARRAYSIZE(modelFileName));

	if (ImGui::Button("Save"))
	{
		l_rhs->m_modelFileName = modelFileName;
	}

	static MaterialDataComponent* selectedComponent = nullptr;

	{
		ImGui::BeginChild("Children MaterialDataComponents", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.3f, 400.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
		{
			for (uint64_t j = 0; j < l_rhs->m_model->meshMaterialPairs.m_count; j++)
			{
				auto l_meshMaterialPair = g_pModuleManager->getAssetSystem()->getMeshMaterialPair(l_rhs->m_model->meshMaterialPairs.m_startOffset + j);

				if (ImGui::Selectable(l_meshMaterialPair->mesh->m_ParentEntity->m_Name.c_str(), selectedComponent == l_meshMaterialPair->material))
				{
					selectedComponent = l_meshMaterialPair->material;
				}
			}
		}
		ImGui::EndChild();
	}

	ImGui::SameLine();

	{
		if (selectedComponent)
		{
			ImGui::BeginChild("MaterialDataComponent Property", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.7f, 400.0f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
			{
				auto l_material = &selectedComponent->m_materialAttributes;

				static float float_min = 0.0f;
				static float float_max = 1.0f;

				static ImVec4 albedo = ImColor(l_material->AlbedoR, l_material->AlbedoG, l_material->AlbedoB, l_material->Alpha);

				if (ImGui::ColorPicker4("Albedo color", (float*)&albedo, ImGuiColorEditFlags_RGB))
				{
					l_material->AlbedoR = albedo.x;
					l_material->AlbedoG = albedo.y;
					l_material->AlbedoB = albedo.z;
					l_material->Alpha = albedo.w;
				}

				const ImVec2 small_slider_size(18, 180);

				auto tt = ImGui::GetCursorPos().x;
				static float metallic = l_material->Metallic;
				if (ImGui::DragFloat("Metallic", &metallic, 0.01f, float_min, float_max))
				{
					l_material->Metallic = metallic;
				}

				static float roughness = l_material->Roughness;
				if (ImGui::DragFloat("Roughness", &roughness, 0.01f, float_min, float_max))
				{
					l_material->Roughness = roughness;
				}

				static float ao = l_material->AO;
				if (ImGui::DragFloat("Ambient Occlusion", &ao, 0.01f, float_min, float_max))
				{
					l_material->AO = ao;
				}

				if (l_rhs->m_visibility == Visibility::Transparent)
				{
					static float thickness = l_material->Thickness;
					if (ImGui::DragFloat("Thickness", &thickness, 0.01f, float_min, float_max))
					{
						l_material->Thickness = thickness;
					}
				}
			}
			ImGui::EndChild();
		}
	}
}

void ImGuiWrapperNS::showLightComponentPropertyEditor(void* rhs)
{
	auto l_rhs = reinterpret_cast<LightComponent*>(rhs);

	ImGui::BeginChild("LightComponent Property", ImVec2(ImGui::GetWindowContentRegionWidth(), 400.0f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	{
		static ImVec4 radiance = ImColor(l_rhs->m_RGBColor.x, l_rhs->m_RGBColor.y, l_rhs->m_RGBColor.z, l_rhs->m_RGBColor.w);

		if (ImGui::ColorPicker4("Radiance color", (float*)&radiance, ImGuiColorEditFlags_RGB))
		{
			l_rhs->m_RGBColor.x = radiance.x;
			l_rhs->m_RGBColor.y = radiance.y;
			l_rhs->m_RGBColor.z = radiance.z;
			l_rhs->m_RGBColor.w = radiance.w;
		}
		static float colorTemperature = l_rhs->m_ColorTemperature;
		if (ImGui::DragFloat("Color temperature", &colorTemperature, 0.01f, 1000.0f, 16000.0f))
		{
			l_rhs->m_ColorTemperature = colorTemperature;
		}
		static float luminousFlux = l_rhs->m_LuminousFlux;
		if (ImGui::DragFloat("Luminous flux", &luminousFlux, 0.01f, 0.0f, 100000.0f))
		{
			l_rhs->m_LuminousFlux = luminousFlux;
		}
		static bool useColorTemperature = l_rhs->m_UseColorTemperature;
		if (ImGui::Checkbox("Use color temperature", &useColorTemperature))
		{
			l_rhs->m_UseColorTemperature = useColorTemperature;
		}

		static float float_min = std::numeric_limits<float>::min();
		static float float_max = std::numeric_limits<float>::max();

		static float pos[4];
		pos[0] = l_rhs->m_Shape.x;
		pos[1] = l_rhs->m_Shape.y;
		pos[2] = l_rhs->m_Shape.z;
		pos[3] = l_rhs->m_Shape.w;

		if (ImGui::DragFloat3("Shape", pos, 0.01f, float_min, float_max))
		{
			l_rhs->m_Shape.x = pos[0];
			l_rhs->m_Shape.y = pos[1];
			l_rhs->m_Shape.z = pos[2];
			l_rhs->m_Shape.w = pos[3];
		}
	}
	ImGui::EndChild();
}

ImVec4 generateButtonColor(const char* name)
{
	auto l_ptr = reinterpret_cast<intptr_t>(name);
	auto l_hue = l_ptr * 4 % 180;
	auto l_RGB = InnoMath::HSVtoRGB(Vec4((float)l_hue, 1.0f, 1.0f, 1.0f));
	return ImVec4(l_RGB.x, l_RGB.y, l_RGB.z, 1.0f);
}

void ImGuiWrapperNS::showConcurrencyProfiler()
{
	if (m_showConcurrencyProfiler)
	{
		auto l_maxThreads = g_pModuleManager->getTaskSystem()->GetTotalThreadsNumber();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
		ImGui::Begin("ConcurrencyProfiler", 0);

		for (uint32_t i = 0; i < l_maxThreads; i++)
		{
			auto l_taskReport = g_pModuleManager->getTaskSystem()->GetTaskReport(i);

			auto l_taskReportCount = l_taskReport.size();

			ImGui::Text("Thread %d", i);
			ImGui::Separator();
			ImGui::BeginGroup();
			if (l_taskReportCount > 0)
			{
				auto l_windowWidth = ImGui::GetWindowContentRegionWidth();

				auto l_relativeStartTime = l_taskReport[0].m_StartTime;
				auto l_relativeEndTime = l_taskReport.currentElement().m_FinishTime;
				auto l_totalDuration = float(l_relativeEndTime - l_relativeStartTime);
				auto l_lastTaskReportButtonPos = l_relativeStartTime;
				uint64_t l_workDuration = 0;

				for (size_t j = 0; j < l_taskReportCount; j++)
				{
					auto l_startTime = l_taskReport[j].m_StartTime;
					auto l_finishTime = l_taskReport[j].m_FinishTime;

					l_workDuration += (l_finishTime - l_startTime);

					auto l_InvalidButtonWidth = ((float)(l_startTime - l_lastTaskReportButtonPos) + 1.0f) * 10.0f / l_totalDuration;
					l_InvalidButtonWidth = InnoMath::clamp(l_InvalidButtonWidth, 0.005f, 20.0f);
					auto l_visibleButtonWidth = ((float)(l_finishTime - l_startTime) + 1.0f) * 10.0f / l_totalDuration;
					l_visibleButtonWidth = InnoMath::clamp(l_visibleButtonWidth, 0.005f, 20.0f);
					ImGui::PushStyleColor(ImGuiCol_Button, generateButtonColor(l_taskReport[j].m_TaskName));
					ImGui::Button("", ImVec2(l_visibleButtonWidth * l_windowWidth, 20));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("%s\nStart %5.3f ms\nEnd %5.3f ms\nDuration %5.3f ms",
							l_taskReport[j].m_TaskName,
							(float)l_startTime / 1000.0f,
							(float)l_finishTime / 1000.0f,
							((float)l_finishTime - (float)l_startTime) / 1000.0f
						);
					}
					ImGui::PopStyleColor(1);
					ImGui::SameLine();
					ImGui::Button("", ImVec2(l_InvalidButtonWidth * l_windowWidth, 20));
					ImGui::SameLine();

					l_lastTaskReportButtonPos = l_finishTime;
				}

				ImGui::Separator();
				auto l_usage = (float)l_workDuration / (float)l_totalDuration;
				ImGui::Text("Usage: %5.3f Report Length: %5.3f ms ", l_usage, (float)l_totalDuration / 1000.0f);
			}
			ImGui::EndGroup();
			ImGui::Separator();
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}
}