#ifndef RENDERITEM_CPP_DEFINED
#define RENDERITEM_CPP_DEFINED

#include "RenderItem.h"

void RenderItem::Init(ComPtr<ID3D12PipelineState> pso, ComPtr<ID3D12Device> device)
{
    pPso = pso;
    objectCB = new UploadBuffer<ObjectConstants>(device.Get(), 1, true);
}


#endif

