#pragma once

/**
 * @brief Gerenciador de memória GPU com otimizações NVIDIA
 * @details Implementa pool de memória, compressão de texturas e gerenciamento eficiente
 */
class CNvidiaMemoryManager
{
public:
    // Tipos de memória
    enum MemoryType
    {
        MEMORY_TYPE_VRAM = 0,         // Memória de vídeo
        MEMORY_TYPE_UNIFIED = 1,      // Memória unificada (se disponível)
        MEMORY_TYPE_PINNED = 2,       // Memória pinada para transferência rápida
    };

    // Informações de bloco de memória
    struct MemoryBlock
    {
        void* pData;
        size_t Size;
        MemoryType Type;
        bool bInUse;
        void* pGPUHandle;
    };

    static CNvidiaMemoryManager* Instance();

    // Inicialização
    bool Initialize(size_t TotalSize);
    void Shutdown();

    // Alocação e liberação
    void* AllocateMemory(size_t size, MemoryType type = MEMORY_TYPE_VRAM);
    bool FreeMemory(void* ptr);
    bool ReallocateMemory(void* ptr, size_t newSize);

    // Transferência de dados
    bool CopyToGPU(void* gpuPtr, const void* cpuPtr, size_t size);
    bool CopyFromGPU(void* cpuPtr, const void* gpuPtr, size_t size);
    bool CopyGPUToGPU(void* dstPtr, const void* srcPtr, size_t size);

    // Otimizações de memória
    bool CompressTexture(void* textureData, size_t& compressedSize);
    bool DecompressTexture(const void* compressedData, void* decompressedData);

    // Estatísticas
    size_t GetTotalMemory() const;
    size_t GetUsedMemory() const;
    size_t GetAvailableMemory() const;
    int GetBlockCount() const;

    // Defragmentação
    void DefragmentMemory();
    float GetFragmentationRatio() const;

    virtual ~CNvidiaMemoryManager();

private:
    CNvidiaMemoryManager();
    
    static CNvidiaMemoryManager* m_pInstance;
    
    std::vector<MemoryBlock> m_MemoryBlocks;
    size_t m_TotalSize;
    size_t m_UsedSize;
    bool m_bInitialized;
    
    bool FindFreeBlock(size_t size, int& outIndex);
    void MergeAdjacentBlocks();
};

#define gNvidiaMemory (CNvidiaMemoryManager::Instance())
