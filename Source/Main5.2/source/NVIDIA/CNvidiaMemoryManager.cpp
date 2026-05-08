#include "stdafx.h"
#include "CNvidiaMemoryManager.h"
#include <algorithm>
#include <iostream>

CNvidiaMemoryManager* CNvidiaMemoryManager::m_pInstance = nullptr;

CNvidiaMemoryManager::CNvidiaMemoryManager()
    : m_TotalSize(0)
    , m_UsedSize(0)
    , m_bInitialized(false)
{
}

CNvidiaMemoryManager::~CNvidiaMemoryManager()
{
    Shutdown();
}

CNvidiaMemoryManager* CNvidiaMemoryManager::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new CNvidiaMemoryManager();
    }
    return m_pInstance;
}

bool CNvidiaMemoryManager::Initialize(size_t TotalSize)
{
    if (m_bInitialized)
        return true;

    m_TotalSize = TotalSize;
    m_UsedSize = 0;
    m_bInitialized = true;

    std::cout << "[NVIDIA Memory] Gerenciador de memória inicializado!" << std::endl;
    std::cout << "[NVIDIA Memory] Tamanho total: " << (TotalSize / 1024 / 1024) << " MB" << std::endl;

    return true;
}

void CNvidiaMemoryManager::Shutdown()
{
    if (!m_bInitialized)
        return;

    for (auto& block : m_MemoryBlocks)
    {
        if (block.pData)
        {
            free(block.pData);
        }
    }

    m_MemoryBlocks.clear();
    m_bInitialized = false;

    std::cout << "[NVIDIA Memory] Gerenciador finalizado!" << std::endl;
}

void* CNvidiaMemoryManager::AllocateMemory(size_t size, MemoryType type)
{
    if (!m_bInitialized || m_UsedSize + size > m_TotalSize)
    {
        std::cerr << "[NVIDIA Memory] Falha ao alocar " << (size / 1024) << " KB" << std::endl;
        return nullptr;
    }

    int freeBlockIndex = -1;
    if (!FindFreeBlock(size, freeBlockIndex))
    {
        return nullptr;
    }

    MemoryBlock block = {};
    block.pData = malloc(size);
    if (!block.pData)
    {
        return nullptr;
    }

    block.Size = size;
    block.Type = type;
    block.bInUse = true;
    block.pGPUHandle = block.pData; // Simplificado

    m_MemoryBlocks.push_back(block);
    m_UsedSize += size;

    return block.pData;
}

bool CNvidiaMemoryManager::FreeMemory(void* ptr)
{
    if (!ptr)
        return false;

    auto it = std::find_if(m_MemoryBlocks.begin(), m_MemoryBlocks.end(),
        [ptr](const MemoryBlock& block) { return block.pData == ptr; });

    if (it == m_MemoryBlocks.end())
    {
        return false;
    }

    m_UsedSize -= it->Size;
    free(it->pData);
    m_MemoryBlocks.erase(it);

    MergeAdjacentBlocks();

    return true;
}

bool CNvidiaMemoryManager::ReallocateMemory(void* ptr, size_t newSize)
{
    if (!ptr)
        return false;

    auto it = std::find_if(m_MemoryBlocks.begin(), m_MemoryBlocks.end(),
        [ptr](const MemoryBlock& block) { return block.pData == ptr; });

    if (it == m_MemoryBlocks.end())
    {
        return false;
    }

    size_t oldSize = it->Size;
    if (oldSize + (newSize - oldSize) > m_TotalSize - m_UsedSize && newSize > oldSize)
    {
        return false;
    }

    void* newData = realloc(ptr, newSize);
    if (!newData)
    {
        return false;
    }

    it->pData = newData;
    m_UsedSize -= oldSize;
    m_UsedSize += newSize;
    it->Size = newSize;
    it->pGPUHandle = newData;

    return true;
}

bool CNvidiaMemoryManager::CopyToGPU(void* gpuPtr, const void* cpuPtr, size_t size)
{
    if (!gpuPtr || !cpuPtr)
        return false;

    memcpy(gpuPtr, cpuPtr, size);
    return true;
}

bool CNvidiaMemoryManager::CopyFromGPU(void* cpuPtr, const void* gpuPtr, size_t size)
{
    if (!cpuPtr || !gpuPtr)
        return false;

    memcpy(cpuPtr, gpuPtr, size);
    return true;
}

bool CNvidiaMemoryManager::CopyGPUToGPU(void* dstPtr, const void* srcPtr, size_t size)
{
    if (!dstPtr || !srcPtr)
        return false;

    memcpy(dstPtr, srcPtr, size);
    return true;
}

bool CNvidiaMemoryManager::CompressTexture(void* textureData, size_t& compressedSize)
{
    // Placeholder para compressão de textura
    // Poderia implementar BC1/BC3/BC7 compression
    return true;
}

bool CNvidiaMemoryManager::DecompressTexture(const void* compressedData, void* decompressedData)
{
    // Placeholder para descompressão
    return true;
}

size_t CNvidiaMemoryManager::GetTotalMemory() const
{
    return m_TotalSize;
}

size_t CNvidiaMemoryManager::GetUsedMemory() const
{
    return m_UsedSize;
}

size_t CNvidiaMemoryManager::GetAvailableMemory() const
{
    return m_TotalSize - m_UsedSize;
}

int CNvidiaMemoryManager::GetBlockCount() const
{
    return (int)m_MemoryBlocks.size();
}

void CNvidiaMemoryManager::DefragmentMemory()
{
    MergeAdjacentBlocks();
}

float CNvidiaMemoryManager::GetFragmentationRatio() const
{
    if (m_MemoryBlocks.empty())
        return 0.0f;

    size_t fragmentedSize = 0;
    for (const auto& block : m_MemoryBlocks)
    {
        if (!block.bInUse)
        {
            fragmentedSize += block.Size;
        }
    }

    return (float)fragmentedSize / m_TotalSize;
}

bool CNvidiaMemoryManager::FindFreeBlock(size_t size, int& outIndex)
{
    for (int i = 0; i < (int)m_MemoryBlocks.size(); ++i)
    {
        if (!m_MemoryBlocks[i].bInUse && m_MemoryBlocks[i].Size >= size)
        {
            outIndex = i;
            return true;
        }
    }

    return (m_UsedSize + size <= m_TotalSize);
}

void CNvidiaMemoryManager::MergeAdjacentBlocks()
{
    for (size_t i = 0; i < m_MemoryBlocks.size() - 1; ++i)
    {
        if (!m_MemoryBlocks[i].bInUse && !m_MemoryBlocks[i + 1].bInUse)
        {
            m_MemoryBlocks[i].Size += m_MemoryBlocks[i + 1].Size;
            m_MemoryBlocks.erase(m_MemoryBlocks.begin() + i + 1);
            --i;
        }
    }
}
