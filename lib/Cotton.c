#include <spvm_native.h>

#include <windows.h>

enum {
  COTTON_C_SIZE_UNIT_PIXEL,
  COTTON_C_SIZE_UNIT_PERCENT,
};

LRESULT CALLBACK WndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp) {

  switch (msg) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd , &ps);

      RECT client_rect;
      GetClientRect(hwnd , &client_rect);
      {
        HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
        SelectObject(hdc, hpen);
        HBRUSH brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
        SelectObject(hdc, brash);
        Rectangle(hdc, 0, 0, client_rect.right, 100);
        DeleteObject(hpen);
        DeleteObject(brash);
      }
      {
        HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0xEE, 0x00, 0x7F));
        SelectObject(hdc, hpen);
        HBRUSH brash = CreateSolidBrush(RGB(0xEE, 0x00, 0x7F));
        SelectObject(hdc, brash);
        Rectangle(hdc, 0, 100, client_rect.right, 200);
        DeleteObject(hpen);
        DeleteObject(brash);
      }
      
      {
        LOGFONT lfFont;
        lfFont.lfHeight     = 40;
        lfFont.lfWidth = lfFont.lfEscapement =
        lfFont.lfOrientation    = 0;
        lfFont.lfWeight     = FW_BOLD;
        lfFont.lfItalic = lfFont.lfUnderline = FALSE;
        lfFont.lfStrikeOut    = FALSE; 
        lfFont.lfCharSet    = SHIFTJIS_CHARSET;
        lfFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
        lfFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        lfFont.lfQuality    = DEFAULT_QUALITY;
        lfFont.lfPitchAndFamily = 0;
        lfFont.lfFaceName[0]    = '\0';
        HFONT hFont = CreateFontIndirect(&lfFont);
        
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
        SetBkMode(hdc , TRANSPARENT);
        TextOut(hdc , 0 , 0 , "Cotton" , lstrlen("Cotton"));
        DeleteObject(hFont);
      }

      EndPaint(hwnd , &ps);
      
      return 0;
    }
  }
  return DefWindowProc(hwnd , msg , wp , lp);
}

int WINAPI WinMain(HINSTANCE hInstance , HINSTANCE hPrevInstance ,
      PSTR lpCmdLine , int nCmdShow ) {
  HWND hwnd;
  MSG msg;
  WNDCLASS winc;

  winc.style    = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc  = WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance    = hInstance;
  winc.hIcon    = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor    = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName  = TEXT("Cotton");

  if (!RegisterClass(&winc)) return -1;

  hwnd = CreateWindow(
      TEXT("Cotton") , TEXT("Cotton") ,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      NULL , NULL , hInstance , NULL
  );

  if (hwnd == NULL) return -1;

  while(GetMessage(&msg , NULL , 0 , 0)) DispatchMessage(&msg);
  return msg.wParam;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL) ;
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
