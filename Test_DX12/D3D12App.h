#ifndef D3D12APP_H_DEFINED
#define D3D12APP_H_DEFINED

#include "Encaps/Render.h"

class D3D12App
{
public:
    D3D12App(HINSTANCE hinstance);
    D3D12App(const D3D12App& rhs) = delete;
    D3D12App& operator=(const D3D12App& rhs) = delete;
    virtual ~D3D12App();

    static D3D12App* GetApp();

    HINSTANCE GetAppInstance() const;
    HWND GetMainWnd() const;
    float GetAspectRatio() const;

    bool Get4xMsaaState() const;
    void Set4xMsaaState(bool value);

    int Run();

    virtual bool Initialize();
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lParam);

protected:
    virtual void CreateRtvAndDsvDescriptorHeaps();
    virtual void OnResize();
    virtual void Update(float dt) {}
    virtual void Draw(float dt) {}

    void BeginDraw();
    void EndDraw();
    
    bool InitMainWindow();
    bool InitDirect3D();
    void CreateCommandObjects();
    void CreateSwapChain();

    void FlushCommandQueue();

    ID3D12Resource* CurrentBackBuffer()const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;
    
protected:
    static D3D12App* m_pInstance;
    HINSTANCE m_appInstance = nullptr;
    HWND m_mainWnd = nullptr;

    bool m_4xMsaaState = false;   
    UINT m_4xMsaaQuality = 0; 
    
    Chrono m_timer;
    
    ComPtr<IDXGIFactory> m_pFactory;
    ComPtr<ID3D12Device> m_pDevice;

    ComPtr<ID3D12Fence> m_pFence;
    UINT m_currentFence = 0;

    ComPtr<ID3D12CommandQueue> m_pCommandQueue;
    ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
    ComPtr<ID3D12GraphicsCommandList> m_pCommandList;

    ComPtr<IDXGISwapChain3> m_pSwapChain;
    
    static const int SwapChainBufferCount = 2;
    ComPtr<ID3D12Resource> m_pSwapChainBuffer[SwapChainBufferCount];
    ComPtr<ID3D12Resource> m_pDepthStencilBuffer;

    ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_pDsvHeap;

    D3D12_VIEWPORT m_screenViewport; 
    D3D12_RECT m_scissorRect;
    
    UINT m_rtvDescriptorSize    = 0;
    UINT m_dsvDescriptorSize    = 0;
    UINT m_cbvSrvDescriptorSize = 0;

    D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    int m_clientWidth = 800;
    int m_clientHeight = 600; 
public:
    
};
#endif
