#ifndef TRIANGLEAPP_H_DEFINED
#define TRIANGLEAPP_H_DEFINED

#include "D3D12App.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

struct ObjectConstants
{
    XMFLOAT4X4 WorldViewProj;

    ObjectConstants()
    {
        XMStoreFloat4x4(&WorldViewProj, XMMatrixIdentity());
    }
};

class TriangleApp : public D3D12App
{
public:
    TriangleApp(HINSTANCE hinstance);
    TriangleApp(const TriangleApp& rhs) = delete;
    TriangleApp& operator=(const TriangleApp& rhs) = delete;
    virtual ~TriangleApp();

    virtual bool Initialize() override;
    
    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildTriangleGeometry();
    void BuildPSO();

private:
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;

    
};

#endif
