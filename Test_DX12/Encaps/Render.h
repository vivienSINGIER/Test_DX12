#ifndef RENDER_H_DEFINED
#define RENDER_H_DEFINED

#include <windows.h>
#include <d3d12.h>
#include "../d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXColors.h>

#include "../Chrono.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#endif
