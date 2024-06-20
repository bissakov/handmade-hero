#include <windows.h>

LRESULT MainWindowCallback(HWND window, UINT message, WPARAM w_param,
                           LPARAM l_param) {
  LRESULT result = 0;

  switch (message) {
    case WM_ACTIVATEAPP: {
      OutputDebugStringW(L"WM_ACTIVATEAPP\n");
      break;
    }

    case WM_SIZE: {
      OutputDebugStringW(L"WM_SIZE\n");
      break;
    }

    case WM_DESTROY: {
      OutputDebugStringW(L"WM_DESTROY\n");
      break;
    }

    case WM_CLOSE: {
      OutputDebugStringW(L"WM_CLOSE\n");

      PostQuitMessage(0);
      break;
    }

    case WM_PAINT: {
      PAINTSTRUCT paint;
      HDC device_context = BeginPaint(window, &paint);

      int x = paint.rcPaint.left;
      int y = paint.rcPaint.top;
      LONG width = paint.rcPaint.right - paint.rcPaint.left;
      LONG height = paint.rcPaint.bottom - paint.rcPaint.top;
      PatBlt(device_context, x, y, width, height, WHITENESS);

      EndPaint(window, &paint);

      break;
    }

    default: {
      result = DefWindowProcW(window, message, w_param, l_param);
      break;
    }
  }

  return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                     LPSTR cmd_line, int show_code) {
  WNDCLASSW window_class = {};

  window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = MainWindowCallback;
  window_class.hInstance = instance;
  window_class.lpszClassName = L"HandmadeHeroWindowClass";

  if (!RegisterClassW(&window_class)) {
    return 1;
  }

  wchar_t window_name[] = L"Handmade Hero";
  unsigned long window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  HWND window = CreateWindowExW(
      0, window_class.lpszClassName, window_name, window_style, CW_USEDEFAULT,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

  if (!window) {
    return 1;
  }

  MSG message;
  while (1) {
    BOOL message_result = GetMessageW(&message, 0, 0, 0);

    if (message_result <= 0) {
      break;
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
  }

  return 0;
}
