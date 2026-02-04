#include <windows.h>

#include "D3D12App.h"
#include "TriangleApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
    TriangleApp app(hInstance);

    app.Initialize();

    app.Run();
    
    return 0;
}