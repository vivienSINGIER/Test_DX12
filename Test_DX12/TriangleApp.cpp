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
    BuildDescriptorHeaps();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildPSO();

    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    BuildConstantBuffers();
    BuildTriangleGeometry();

    
    // Execute the initialization commands.
    m_pCommandList->Close();
    ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    float x = 0.0f;
    float z = 20.0f;
    float y = 0.0f;
    
    // Build the view matrix.
    XMVECTOR pos = XMVectorZero();
    XMVECTOR target = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);

    XMMATRIX world = XMMatrixTranslation(x, y, z);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world*view*proj;

    // Update the constant buffer with the latest worldViewProj matrix.
    ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
    
    m_pObjectCB->CopyData(0, objConstants);

    return true;
}

void TriangleApp::Draw(float dt)
{
    // Set the used descriptor heaps
    ID3D12DescriptorHeap* desciptorHeaps[] = { m_cbvHeap.Get() };
    m_pCommandList->SetDescriptorHeaps(_countof(desciptorHeaps), desciptorHeaps);
    
    m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
    m_pCommandList->SetPipelineState(m_pPSO.Get());
    
    m_pCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = m_pGeo->VertexBufferView();
    D3D12_INDEX_BUFFER_VIEW indexBufferView = m_pGeo->IndexBufferView();
    m_pCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    m_pCommandList->IASetIndexBuffer(&indexBufferView);
    
    m_pCommandList->SetGraphicsRootConstantBufferView(0, m_pObjectCB->Resource()->GetGPUVirtualAddress());
    
    m_pCommandList->DrawIndexedInstanced(m_pGeo->DrawArgs["triangle"].IndexCount,
        1, 0, 0, 0);
    
}

void TriangleApp::Update(float dt)
{
    
}

void TriangleApp::OnResize()
{
    D3D12App::OnResize();
    float r = GetAspectRatio();
    
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, GetAspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
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
    m_pObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_pDevice.Get(), 1, true);
}

void TriangleApp::BuildRootSignature()
{
    const int count = 1;
    CD3DX12_ROOT_PARAMETER slotRootParameters[count];
    slotRootParameters[0].InitAsConstantBufferView(0);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDes(1, slotRootParameters,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(&rootSigDes, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if (errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*) errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(m_pDevice->CreateRootSignature(0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_pRootSignature)));
}

void TriangleApp::BuildShadersAndInputLayout()
{
    m_pVsByteCode = d3dUtil::CompileShader(L"Shaders/color.hlsl", nullptr, "VS", "vs_5_0");
    m_pPsByteCode = d3dUtil::CompileShader(L"Shaders/color.hlsl", nullptr, "PS", "ps_5_0");
    
    m_inputLayout.push_back(
         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        );
    m_inputLayout.push_back(
         {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, 
        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        );
}

void TriangleApp::BuildTriangleGeometry()
{
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 0.5f, 0.0f), XMFLOAT4(Colors::Red) },
        { XMFLOAT3(1.0f, 0.5f, 0.0f), XMFLOAT4(Colors::Blue) },
        { XMFLOAT3(0.0f, -0.5f, 0.0f), XMFLOAT4(Colors::Green) },
    };

    uint16_t indices[] =
    {
        0, 1, 2,    
        //0, 2, 1    
    };
    
    const UINT64 vbByteSize = 3 * sizeof(Vertex);
    const UINT64 ibByteSize = 3 * sizeof(uint16_t);

    m_pGeo = std::make_unique<MeshGeometry>();
    m_pGeo->Name = "Triangle";
    
    m_pGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_pDevice.Get(),
     m_pCommandList.Get(), vertices, vbByteSize, m_pGeo->VertexBufferUploader);
    
    m_pGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_pDevice.Get(),
        m_pCommandList.Get(), indices, ibByteSize, m_pGeo->IndexBufferUploader);

    m_pGeo->VertexByteStride = sizeof(Vertex);
    m_pGeo->VertexBufferByteSize = vbByteSize;
    m_pGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
    m_pGeo->IndexBufferByteSize = ibByteSize;

    SubmeshGeometry submesh;
    submesh.IndexCount = 3;
    submesh.StartIndexLocation = 0;
    submesh.BaseVertexLocation = 0;

    m_pGeo->DrawArgs["triangle"] = submesh;
}

void TriangleApp::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
    psoDesc.pRootSignature = m_pRootSignature.Get();
    psoDesc.VS = 
    { 
        reinterpret_cast<BYTE*>(m_pVsByteCode->GetBufferPointer()), 
        m_pVsByteCode->GetBufferSize() 
    };
    psoDesc.PS = 
    { 
        reinterpret_cast<BYTE*>(m_pPsByteCode->GetBufferPointer()), 
        m_pPsByteCode->GetBufferSize() 
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(m_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPSO)));
}


#endif
