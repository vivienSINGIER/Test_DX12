#include <windows.h>

#include "D3D12App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    D3D12App app(hInstance);

    app.Initialize();

    app.Run();
    
    return 0;
}