#include <spvm_native.h>

#include <windows.h>

//ビットマップバイト幅の算出マクロ
#ifndef WIDTHBYTES
#define WIDTHBYTES(bits) (((bits)+31)/32*4)
#endif//WIDTHBYTES

typedef struct{
  unsigned char *data;
  unsigned int width;
  unsigned int height;
  unsigned int ch;
} BITMAPDATA_t;

enum {
  COTTON_C_SIZE_UNIT_PIXEL,
  COTTON_C_SIZE_UNIT_PERCENT,
};

#define SIGNATURE_NUM 8

LRESULT CALLBACK WndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp) {
  
  static HWND div = NULL;
  
  BITMAP bitmap = {0};
  switch (msg) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {

  		div = CreateWindow(
  			TEXT("BUTTON") , NULL ,
  			WS_CHILD | WS_VISIBLE | SS_CENTER | BS_OWNERDRAW,
  			500 , 500 , 200 , 45 ,
  			hwnd , (HMENU)1 ,
  			((LPCREATESTRUCT)(lp))->hInstance , NULL
  		);
  		
  		

		  return 0;
    }
    case WM_DRAWITEM: {
      
      LPDRAWITEMSTRUCT draw_item = (LPDRAWITEMSTRUCT)lp;
      HDC hdc = draw_item->hDC;
      
      SelectObject(hdc , CreateHatchBrush(HS_DIAGCROSS , RGB(0xFF , 0 , 0)));
      Rectangle(hdc , 0 , 0 , draw_item->rcItem.right, draw_item->rcItem.bottom);
  		DeleteObject(SelectObject(hdc , GetStockObject(WHITE_BRUSH)));      

      return TRUE;
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine ,int nCmdShow ) {
  
  // Window Class
  WNDCLASS winc;
  winc.style    = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc  = WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance    = hInstance;
  winc.hIcon    = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor    = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName  = TEXT("main_window");

  // Register Window Class
  if (!RegisterClass(&winc)) return -1;
  
  // Create Main Window
  HWND hwnd = CreateWindow(
      TEXT("main_window") , TEXT("Cotton") ,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      NULL , NULL , hInstance , NULL
  );

  if (hwnd == NULL) return -1;

  MSG msg;
  while(GetMessage(&msg , NULL , 0 , 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL) ;
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
