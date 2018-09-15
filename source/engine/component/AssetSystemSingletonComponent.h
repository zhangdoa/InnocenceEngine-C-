#pragma once
#include "BaseComponent.h"

#if defined (INNO_RENDERER_OPENGL)
#include "../component/GLMeshDataComponent.h"
#include "../component/GLTextureDataComponent.h"
#define MeshDataComponent GLMeshDataComponent
#define TextureDataComponent GLTextureDataComponent
#elif defined (INNO_RENDERER_DX)
#include "../component/DXMeshDataComponent.h"
#include "../component/DXTextureDataComponent.h"
#define MeshDataComponent DXMeshDataComponent
#define TextureDataComponent DXTextureDataComponent
#endif

#include "../common/InnoConcurrency.h"
class AssetSystemSingletonComponent : public BaseComponent
{
public:
	~AssetSystemSingletonComponent() {};

	static AssetSystemSingletonComponent& getInstance()
	{
		static AssetSystemSingletonComponent instance;
		return instance;
	}

    std::unordered_map<std::string, int> m_supportedTextureType = { {"png", 0} };
    std::unordered_map<std::string, int> m_supportedModelType = { {"obj", 0}, {"innoModel", 0} };
    std::unordered_map<std::string, int> m_supportedShaderType = { {"sf", 0} };

	std::unordered_map<meshID, MeshDataComponent*> m_meshMap;
	std::unordered_map<textureID, TextureDataComponent*> m_textureMap;

	ThreadSafeQueue<MeshDataComponent*> m_uninitializedMeshComponents;
	ThreadSafeQueue<TextureDataComponent*> m_uninitializedTextureComponents;

	meshID m_UnitLineTemplate;
	meshID m_UnitQuadTemplate;
	meshID m_UnitCubeTemplate;
	meshID m_UnitSphereTemplate;

	textureID m_basicNormalTemplate;
	textureID m_basicAlbedoTemplate;
	textureID m_basicMetallicTemplate;
	textureID m_basicRoughnessTemplate;
	textureID m_basicAOTemplate;

	std::unordered_map<std::string, modelMap> m_loadedModelMap;
	std::unordered_map<std::string, texturePair> m_loadedTextureMap;

    const std::string m_textureRelativePath = std::string{"..//res//textures//"};
    const std::string m_modelRelativePath = std::string{"..//res//models//"};
    const std::string m_shaderRelativePath = std::string{"..//res//shaders//"};

private:
	AssetSystemSingletonComponent() {};
};
