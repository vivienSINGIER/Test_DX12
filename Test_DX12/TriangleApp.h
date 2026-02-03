#ifndef TRIANGLEAPP_H_DEFINED
#define TRIANGLEAPP_H_DEFINED
#include <memory>

#include "D3D12App.h"

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
    void BuildBoxGeometry();
    void BuildPSO();

private:
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap = nullptr;

    std::unique_ptr<UploadBuffer<>>
};

#endif
