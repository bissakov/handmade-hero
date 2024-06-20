#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nShowCmd) {

  MessageBoxW(0, L"Hello, World!", L"Handmade Hero",
              MB_OK | MB_ICONINFORMATION);

  return 0;
}
