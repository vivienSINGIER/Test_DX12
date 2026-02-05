#ifndef TRIANGLEAPP_H_DEFINED
#define TRIANGLEAPP_H_DEFINED

#include "D3D12App.h"
#include "UploadBuffer.h"
#include "d3dUtil.h"
#include "MathHelper.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class TriangleApp : public D3D12App
{
public:
    TriangleApp(HINSTANCE hinstance);
    TriangleApp(const TriangleApp& rhs) = delete;
    TriangleApp& operator=(const TriangleApp& rhs) = delete;
    virtual ~TriangleApp();

    virtual bool Initialize() override;
    virtual void Draw(float dt) override;
    virtual void Update(float dt) override;
    
    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildTriangleGeometry();
    void BuildPSO();

private:

    ComPtr<ID3D12RootSignature> m_pRootSignature = nullptr;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;
    ComPtr<ID3D12PipelineState> m_pPSO = nullptr;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayout;
    
    std::unique_ptr<UploadBuffer<ObjectConstants>> m_pObjectCB = nullptr;
    std::unique_ptr<MeshGeometry> m_pGeo = nullptr;

    ComPtr<ID3DBlob> m_pVsByteCode = nullptr;
    ComPtr<ID3DBlob> m_pPsByteCode = nullptr;

    XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();
};

#endif
