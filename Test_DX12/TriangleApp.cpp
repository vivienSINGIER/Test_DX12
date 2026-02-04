#ifndef TRIANGLEAPP_CPP_DEFINED
#define TRIANGLEAPP_CPP_DEFINED

#include "TriangleApp.h"

#include "d3dUtil.h"

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
    BuildTriangleGeometry();
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

void TriangleApp::BuildRootSignature()
{
}

void TriangleApp::BuildShadersAndInputLayout()
{
    D3D12_INPUT_ELEMENT_DESC desc1[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
}

void TriangleApp::BuildTriangleGeometry()
{
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 0.5f, 0.0f), XMFLOAT4(Colors::Red) },
        { XMFLOAT3(1.0f, 0.5f, 0.0f), XMFLOAT4(Colors::Blue) },
        { XMFLOAT3(0.0f, -0.5f, -1.0f), XMFLOAT4(Colors::Green) },
    };
    
    const UINT64 vbByteSize = 3 * sizeof(Vertex);
    ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_pDevice.Get(),
     m_pCommandList.Get(), vertices, vbByteSize, VertexBufferUploader);
}

void TriangleApp::BuildPSO()
{
}


#endif
