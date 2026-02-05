#ifndef D3D12APP_CPP_DEFINED
#define D3D12APP_CPP_DEFINED

#include "D3D12App.h"

#include <corecrt_wstdio.h>

#include "d3dUtil.h"

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return D3D12App::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

D3D12App* D3D12App::m_pInstance = nullptr;

D3D12App* D3D12App::GetApp()
{
    return m_pInstance;
}

D3D12App::D3D12App(HINSTANCE hinstance) : m_appInstance(hinstance)
{
    m_pInstance = this;
}

D3D12App::~D3D12App()
{
    if(m_pDevice != nullptr)
        FlushCommandQueue();
}

HINSTANCE D3D12App::GetAppInstance() const
{
    return m_appInstance;
}

HWND D3D12App::GetMainWnd() const
{
    return m_mainWnd;
}

float D3D12App::GetAspectRatio() const
{
    return m_clientWidth / m_clientHeight;
}

bool D3D12App::Get4xMsaaState() const
{
    return m_4xMsaaState;
}

void D3D12App::Set4xMsaaState(bool value)
{
    if(m_4xMsaaState != value)
    {
        m_4xMsaaState = value;

        // Recreate the swapchain and buffers with new multisample settings.
        CreateSwapChain();
        OnResize();
    }
}

int D3D12App::Run()
{
    MSG msg = {0};
 
    m_timer.Start();

    while(msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        while (PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        float dt = m_timer.Reset();
        
        Update(dt);

        BeginDraw();
        Draw(dt);
        EndDraw();

        FlushCommandQueue();
    }

    return (int)msg.wParam;
}

bool D3D12App::Initialize()
{
    if(!InitMainWindow())
        return false;

    if(!InitDirect3D())
        return false;

    // Do the initial resize code.
    OnResize();
    ShowWindow(m_mainWnd, SW_SHOW);
    UpdateWindow(m_mainWnd);

    return true;
}

LRESULT D3D12App::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYUP:
        if(wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        else if((int)wParam == VK_F2)
            Set4xMsaaState(!m_4xMsaaState);

        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void D3D12App::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    m_pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_pRtvHeap.GetAddressOf()));

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;

    m_pDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_pDsvHeap.GetAddressOf()));
}

void D3D12App::OnResize()
{
    FlushCommandQueue();

    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		m_pSwapChainBuffer[i].Reset();
    m_pDepthStencilBuffer.Reset();
	
	// Resize the swap chain.
    m_pSwapChain->ResizeBuffers(
		SwapChainBufferCount, 
		m_clientWidth, m_clientHeight, 
		mBackBufferFormat, 
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
    
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_pRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainBuffer[i]));
		m_pDevice->CreateRenderTargetView(m_pSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += m_rtvDescriptorSize;
	}

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_clientWidth;
    depthStencilDesc.Height = m_clientHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = mDepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProperties;
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;
    
    m_pDevice->CreateCommittedResource(
        &heapProperties,
		D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_pDepthStencilBuffer.GetAddressOf()));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
    m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());
    
    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pDepthStencilBuffer.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    
    // Transition the resource from its initial state to be used as a depth buffer.
	m_pCommandList->ResourceBarrier(1, &barrier);
	
    // Execute the resize commands.
    m_pCommandList->Close();
    ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width    = static_cast<float>(m_clientWidth);
	m_screenViewport.Height   = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

    m_scissorRect = { 0, 0, m_clientWidth, m_clientHeight };
}

void D3D12App::BeginDraw()
{
    m_pCommandAllocator->Reset();
    m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);

    D3D12_RESOURCE_BARRIER rbarrier;
    rbarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rbarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rbarrier.Transition.pResource = CurrentBackBuffer();
    rbarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    rbarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rbarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_pCommandList->ResourceBarrier(1, &rbarrier);
    
    m_pCommandList->ClearRenderTargetView(
        CurrentBackBufferView(),
        DirectX::Colors::LightSteelBlue, 0, nullptr);
    m_pCommandList->ClearDepthStencilView(DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        1.0f, 0, 0, nullptr
        );
    
    m_pCommandList->RSSetViewports(1, &m_screenViewport);
    m_pCommandList->RSSetScissorRects(1, &m_scissorRect);
    
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DepthStencilView();
    m_pCommandList->OMSetRenderTargets(1, &rtvHandle,
        true, &dsvHandle);

}

void D3D12App::EndDraw()
{
    D3D12_RESOURCE_BARRIER rbarrier2;
    rbarrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    rbarrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    rbarrier2.Transition.pResource = CurrentBackBuffer();
    rbarrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    rbarrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    rbarrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_pCommandList->ResourceBarrier(1, &rbarrier2);

    m_pCommandList->Close();

    ID3D12CommandList* cmdsLists[] = { m_pCommandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    HRESULT hr = m_pSwapChain->Present(0, 0);
    ThrowIfFailed(hr);
}

bool D3D12App::InitMainWindow()
{
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = m_appInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc))
    {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS)  // 1410 is OK
        {
            MessageBox(0, L"RegisterClass Failed.", 0, 0);
            return false;
        }

    }
    
    m_mainWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Dx12 App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, m_clientWidth, m_clientHeight,
        nullptr,
        nullptr,
        m_appInstance,
        nullptr
    );

    if( !m_mainWnd )
    {
        DWORD error = GetLastError();  // Get error IMMEDIATELY
        wchar_t buf[256];
        swprintf_s(buf, L"CreateWindowEx failed. Error code: %d\nHandle: 0x%p", error, m_mainWnd);
        MessageBox(0, buf, L"Window Creation Error", 0);
        return false;
    }

    return true;
}

bool D3D12App::InitDirect3D()
{
#if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif
    
    CreateDXGIFactory(IID_PPV_ARGS(&m_pFactory));
    
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pDevice));
    
    m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));

    m_rtvDescriptorSize      = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize      = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    m_cbvSrvDescriptorSize   = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = mBackBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    m_pDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels));

    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;

    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void D3D12App::CreateCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));
    m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
    m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList) );

    m_pCommandList->Close();
}

void D3D12App::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_clientWidth;
    sd.BufferDesc.Height = m_clientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = m_mainWnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ComPtr<IDXGISwapChain> tempSwapChain;
    m_pFactory->CreateSwapChain(m_pCommandQueue.Get(), &sd, tempSwapChain.GetAddressOf());

    tempSwapChain.As(&m_pSwapChain);
}

void D3D12App::FlushCommandQueue()
{
    m_currentFence++;
    m_pCommandQueue->Signal(m_pFence.Get(), m_currentFence);
    
    if (m_pFence->GetCompletedValue() < m_currentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        
        m_pFence->SetEventOnCompletion(m_currentFence, eventHandle);
        
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* D3D12App::CurrentBackBuffer() const
{
    return m_pSwapChainBuffer[m_pSwapChain->GetCurrentBackBufferIndex()].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
         m_pRtvHeap->GetCPUDescriptorHandleForHeapStart(),
         m_pSwapChain->GetCurrentBackBufferIndex(),
         m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12App::DepthStencilView() const
{
    return m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();
}


#endif

