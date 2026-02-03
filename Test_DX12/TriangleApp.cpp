#ifndef TRIANGLEAPP_CPP_DEFINED
#define TRIANGLEAPP_CPP_DEFINED

#include "TriangleApp.h"

TriangleApp::TriangleApp(HINSTANCE hinstance) : D3D12App(hinstance)
{
}

TriangleApp::~TriangleApp()
{
}

bool TriangleApp::Initialize()
{
    if (D3D12App::Initialize() == false)
        return false;

    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    BuildDescriptorHeaps();
    BuildConstantBuffers();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildBoxGeometry();
    BuildPSO();

    // Execute the initialization commands.
    m_pCommandList->Close();
    ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();
}

void TriangleApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    m_pDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_cbvHeap));
}

void TriangleApp::BuildConstantBuffers()
{
    
}


#endif
