/**
 * @file ExampleNVIDIAUsage.cpp
 * @brief Exemplos completos de como usar o sistema NVIDIA no MuOnline
 * 
 * Este arquivo demonstra padrões práticos de uso do sistema NVIDIA.
 * Copie e adapte os exemplos para seu código.
 * 
 * NOTA: Este é um arquivo de REFERÊNCIA. Não precisa compilar.
 * Use os padrões aqui como base para sua implementação.
 */

#include "stdafx.h"
#include "NVIDIA/CNvidiaSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================================
// EXEMPLO 1: Inicialização Básica
// ============================================================================

bool InitializeNVIDIA()
{
	// Inicializar o sistema completo de NVIDIA
	if (!gNvidiaSystem->Initialize())
	{
		std::cerr << "Falha ao inicializar NVIDIA System!" << std::endl;
		return false;
	}

	// Imprimir informações do sistema
	gNvidiaSystem->PrintSystemInfo();

	return true;
}

void ShutdownNVIDIA()
{
	if (gNvidiaSystem->IsInitialized())
	{
		gNvidiaSystem->Shutdown();
	}
}

// ============================================================================
// EXEMPLO 2: Renderizar Mesh Simples
// ============================================================================

void RenderMeshExample(
	GLuint VAO, 
	GLuint IndexBuffer, 
	GLuint ShaderProgram,
	const glm::vec3& position,
	const glm::vec3& scale,
	int indexCount)
{
	if (!gNvidiaGPU->IsGPURenderingEnabled())
	{
		return;  // GPU rendering desativado
	}

	// Construir matriz de modelo
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model = glm::scale(model, scale);

	// Renderizar usando NVIDIA
	gNvidiaRender->RenderMesh(VAO, IndexBuffer, ShaderProgram, model, indexCount);
}

// ============================================================================
// EXEMPLO 3: Renderizar Character com Múltiplas Meshes
// ============================================================================

void RenderCharacterWithNVIDIA(
	BMD* characterModel, 
	const glm::vec3& position,
	const glm::vec3& rotation,
	float scale)
{
	if (!characterModel || !gNvidiaGPU->IsGPURenderingEnabled())
	{
		return;
	}

	// Criar matriz de modelo
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	
	// Aplicar rotação
	modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	
	// Aplicar escala
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));

	// Renderizar cada mesh
	for (int i = 0; i < characterModel->NumMeshs; ++i)
	{
		Mesh_t* mesh = &characterModel->Meshs[i];
		
		if (mesh->VAO != 0 && mesh->EBO != 0)
		{
			// Renderizar mesh
			gNvidiaRender->RenderMesh(
				mesh->VAO,
				mesh->EBO,
				0,  // Ajustar com seu shader program
				modelMatrix,
				mesh->NumTriangles * 3
			);
		}
	}
}

// ============================================================================
// EXEMPLO 4: Renderização com Instancing (Múltiplas Entidades)
// ============================================================================

void RenderInstancedModels(
	GLuint VAO,
	GLuint IndexBuffer,
	GLuint ShaderProgram,
	const glm::vec3* positions,
	int count,
	int indexCount)
{
	if (!gNvidiaGPU->IsGPURenderingEnabled() || count <= 0)
	{
		return;
	}

	// Construir matrizes de modelo para cada instância
	glm::mat4* modelMatrices = new glm::mat4[count];
	
	for (int i = 0; i < count; ++i)
	{
		modelMatrices[i] = glm::mat4(1.0f);
		modelMatrices[i] = glm::translate(modelMatrices[i], positions[i]);
	}

	// Renderizar com instancing
	gNvidiaRender->RenderMeshInstanced(
		VAO,
		IndexBuffer,
		ShaderProgram,
		modelMatrices,
		count,
		indexCount
	);

	delete[] modelMatrices;
}

// ============================================================================
// EXEMPLO 5: Aplicar Efeitos Pós-Processamento
// ============================================================================

void ApplyScreenSpaceAmbientOcclusion(
	GLuint normalTexture,
	GLuint depthTexture)
{
	if (!gNvidiaSystem->IsInitialized())
	{
		return;
	}

	const auto& renderConfig = gNvidiaRender->GetRenderConfig();
	
	if (renderConfig.EnableSSAO)
	{
		gNvidiaRender->ApplySSAO(normalTexture, depthTexture);
	}
}

void ApplyScreenSpaceReflections(
	GLuint normalTexture,
	GLuint depthTexture,
	GLuint reflectionTexture)
{
	if (!gNvidiaSystem->IsInitialized())
	{
		return;
	}

	const auto& renderConfig = gNvidiaRender->GetRenderConfig();
	
	if (renderConfig.EnableSSR)
	{
		gNvidiaRender->ApplySSR(normalTexture, depthTexture, reflectionTexture);
	}
}

// ============================================================================
// EXEMPLO 6: Monitoramento de Performance e Memória
// ============================================================================

void DisplayNVIDIAPerformanceHUD()
{
	if (!gNvidiaSystem->IsInitialized())
	{
		return;
	}

	// Obter estatísticas
	const auto& stats = gNvidiaGPU->GetRenderStats();
	int gpuCount = gNvidiaGPU->GetGPUCount();
	size_t usedMemory = gNvidiaMemory->GetUsedMemory();
	size_t totalMemory = gNvidiaMemory->GetTotalMemory();

	// Formatação para debug
	char debugText[512];
	sprintf_s(debugText, sizeof(debugText),
		"[NVIDIA System]\n"
		"GPUs: %d\n"
		"GPU Usage: %.1f%%\n"
		"Memory: %llu / %llu MB (%.1f%%)\n"
		"Frames: %llu\n"
		"FPS: %.1f",
		gpuCount,
		stats.GPUUsage,
		usedMemory / 1024 / 1024,
		totalMemory / 1024 / 1024,
		stats.MemoryUsage,
		stats.FramesRendered,
		stats.FPS
	);

	// Renderizar no HUD (ajustar conforme sua engine)
	// g_pUIManager->RenderDebugText(debugText, 10, 10);
}

// ============================================================================
// EXEMPLO 7: Gerenciamento de Memória GPU
// ============================================================================

class GPUMeshCache
{
private:
	struct CachedMesh
	{
		void* cpuData;
		size_t cpuSize;
		void* gpuData;
		bool bCached;
	};

	std::map<int, CachedMesh> m_cache;

public:
	bool CacheMeshOnGPU(int meshID, const void* meshData, size_t size)
	{
		// Alocar memória na GPU
		void* gpuMemory = gNvidiaMemory->AllocateMemory(size);
		
		if (!gpuMemory)
		{
			std::cerr << "Falha ao alocar memória GPU para mesh " << meshID << std::endl;
			return false;
		}

		// Copiar dados para GPU
		if (!gNvidiaMemory->CopyToGPU(gpuMemory, meshData, size))
		{
			gNvidiaMemory->FreeMemory(gpuMemory);
			return false;
		}

		// Armazenar em cache
		CachedMesh cached = {};
		cached.gpuData = gpuMemory;
		cached.cpuSize = size;
		cached.bCached = true;

		m_cache[meshID] = cached;

		std::cout << "Mesh " << meshID << " cached on GPU (" 
				  << (size / 1024 / 1024) << " MB)" << std::endl;

		return true;
	}

	void FreeMeshCache(int meshID)
	{
		auto it = m_cache.find(meshID);
		if (it != m_cache.end())
		{
			gNvidiaMemory->FreeMemory(it->second.gpuData);
			m_cache.erase(it);
		}
	}

	void ClearAllCache()
	{
		for (auto& pair : m_cache)
		{
			gNvidiaMemory->FreeMemory(pair.second.gpuData);
		}
		m_cache.clear();

		// Desfragmentar memória
		gNvidiaMemory->DefragmentMemory();
	}

	size_t GetCacheSize() const
	{
		return gNvidiaMemory->GetUsedMemory();
	}
};

// ============================================================================
// EXEMPLO 8: Configuração Personalizada
// ============================================================================

void ConfigureNVIDIAForMaxPerformance()
{
	if (!gNvidiaSystem->IsInitialized())
	{
		return;
	}

	// Configurar para renderização de alta performance
	CNvidiaRenderSystem::RenderConfig config;
	config.Type = CNvidiaRenderSystem::RENDER_TYPE_DEFERRED;
	config.EnableSSAO = true;
	config.EnableSSR = true;
	config.EnableMotionBlur = true;
	config.EnableTAA = true;
	config.EnableAsyncCompute = true;
	config.MaxLights = 64;
	config.ShadowResolution = 4096;

	gNvidiaRender->SetRenderConfig(config);
}

void ConfigureNVIDIAForQuality()
{
	if (!gNvidiaSystem->IsInitialized())
	{
		return;
	}

	// Configurar para máxima qualidade
	CNvidiaRenderSystem::RenderConfig config;
	config.Type = CNvidiaRenderSystem::RENDER_TYPE_DEFERRED;
	config.EnableSSAO = true;
	config.EnableSSR = true;
	config.EnableMotionBlur = true;
	config.EnableTAA = true;
	config.EnableAsyncCompute = true;
	config.MaxLights = 128;
	config.ShadowResolution = 8192;

	gNvidiaRender->SetRenderConfig(config);
}

// ============================================================================
// EXEMPLO 9: Detecção de Capacidades
// ============================================================================

void PrintNVIDIACapabilities()
{
	if (!gNvidiaGPU->IsNvidiaSupported())
	{
		std::cout << "NVIDIA GPU não suportada!" << std::endl;
		return;
	}

	std::cout << "\n=== NVIDIA System Capabilities ===" << std::endl;

	// Informações da GPU
	int gpuCount = gNvidiaGPU->GetGPUCount();
	std::cout << "GPUs Detectadas: " << gpuCount << std::endl;

	for (int i = 0; i < gpuCount; ++i)
	{
		const auto& gpuInfo = gNvidiaGPU->GetGPUInfo(i);
		
		std::cout << "\nGPU #" << i << std::endl;
		std::cout << "  Nome: " << gpuInfo.DeviceName << std::endl;
		std::cout << "  Memória Total: " << (gpuInfo.TotalMemory / 1024 / 1024) << " MB" << std::endl;
		std::cout << "  Suportada: " << (gpuInfo.IsSupported ? "SIM" : "NÃO") << std::endl;
		std::cout << "  Compute Capability: " << (gpuInfo.ComputeCapability / 10) 
				  << "." << (gpuInfo.ComputeCapability % 10) << std::endl;
		std::cout << "  Clock Rate: " << gpuInfo.ClockRate << " MHz" << std::endl;
		std::cout << "  Max Threads/Block: " << gpuInfo.MaxThreadsPerBlock << std::endl;
	}

	// Capacidades de renderização
	std::cout << "\nRecursos de Renderização:" << std::endl;
	std::cout << "  Ray Tracing: " << (gNvidiaGPU->IsNvidiaSupported() ? "SIM" : "NÃO") << std::endl;
	std::cout << "  CUDA Disponível: " << (gNvidiaGPU->IsCUDAAvailable() ? "SIM" : "NÃO") << std::endl;
	std::cout << "  GPU Rendering: " << (gNvidiaGPU->IsGPURenderingEnabled() ? "ATIVO" : "INATIVO") << std::endl;

	std::cout << "\n================================\n" << std::endl;
}
