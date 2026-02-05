#ifndef RENDERITEM_H_DEFINED
#define RENDERITEM_H_DEFINED

#include "Render.h"
#include "../d3dUtil.h"

#include "Transform.h"

struct ObjectConstants
{
    XMFLOAT4X4 worldViewProj = MathHelper::Identity4x4();
};

class RenderItem
{
public:
    MeshGeometry* pGeo = nullptr;
    UploadBuffer<ObjectConstants>* objectCB = nullptr;

    ComPtr<ID3D12PipelineState> pPso;

    Transform transform;

    RenderItem() = default;
    void Init(ComPtr<ID3D12PipelineState> pso, ComPtr<ID3D12Device> device);
};

#endif
