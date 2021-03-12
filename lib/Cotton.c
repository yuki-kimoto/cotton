#include <spvm_native.h>

#include <windows.h>

int WINAPI WinMain(
    HINSTANCE hInstance ,
    HINSTANCE hPrevInstance ,
    PSTR lpCmdLine ,
    int nCmdShow ) {
  HWND hwnd;
  WNDCLASS winc;

  winc.style    = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc  = DefWindowProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance    = hInstance;
  winc.hIcon    = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor    = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName  = TEXT("Cotton");

  if (!RegisterClass(&winc)) return 0;

  hwnd = CreateWindow(
      TEXT("Cotton") , TEXT("Cotton") ,
      WS_OVERLAPPEDWINDOW ,
      100 , 100 , 200 , 200 , NULL , NULL ,
      hInstance , NULL
  );

  if (hwnd == NULL) return 0;

  ShowWindow(hwnd , SW_SHOW);
  MessageBox(NULL , TEXT("Cotton") ,
      TEXT("Cotton") , MB_ICONINFORMATION);

  return 0;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL) ;
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
