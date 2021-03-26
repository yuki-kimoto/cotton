#include <spvm_native.h>

#include <windows.h>

typedef struct cotton_win COTTON_WIN;
struct cotton_win {
  int32_t dummy;
};

typedef struct cotton_win_new_args COTTON_WIN_NEW_ARGS;
struct cotton_win_new_args {
};

COTTON_WIN* COTTON_WIN_new() {
  COTTON_WIN* cotton = calloc(1, sizeof(COTTON_WIN));
  
  return cotton;
}

COTTON_WIN COTTON_WIN_free(COTTON_WIN* cotton) {
  free(cotton);
}

typedef struct cotton_win_new_main_window_args COTTON_WIN_NEW_MAIN_WINDOW_ARGS;
struct cotton_win_new_main_window_args {
  LPCTSTR title;
  COTTON_WIN* cotton;
};

typedef struct cotton_win_new_block_window_args COTTON_WIN_NEW_BLOCK_WINDOW_ARGS;
struct cotton_win_new_block_window_args {
  HWND parent_window_handle;
  int32_t window_id;
};

HWND COTTON_WIN_new_main_window(COTTON_WIN* cotton, COTTON_WIN_NEW_MAIN_WINDOW_ARGS* args);
HWND COTTON_WIN_new_block_window(COTTON_WIN* cotton, COTTON_WIN_NEW_BLOCK_WINDOW_ARGS* args);

HWND COTTON_WIN_new_block_window(COTTON_WIN* cotton, COTTON_WIN_NEW_BLOCK_WINDOW_ARGS* args) {

  // Create block. Now block is implemented as owner draw button
  LPCTSTR window_class_name = TEXT("BUTTON");
  LPCTSTR window_title = NULL;
  DWORD window_style = WS_CHILD | WS_VISIBLE | BS_OWNERDRAW;
  int window_x = 0;
  int window_y = 0;
  int window_width = 0;
  int window_heigth = 0;
  HWND window_parent_window_handle = args->parent_window_handle;
  HMENU window_id = (HMENU)(intptr_t)args->window_id;
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  LPVOID window_create_lparam = NULL;
  HWND block = CreateWindow(
    window_class_name, window_title,
    window_style,
    window_x, window_y, window_width, window_heigth,
    window_parent_window_handle, window_id,
    instance_handle, window_create_lparam
  );
  
  return block;
}

typedef struct cotton_win_block COTTON_WIN_BLOCK;
struct cotton_win_block {
  int32_t font_size;
  int32_t color;
  int32_t font_weight;
};

COTTON_WIN_BLOCK* COTTON_WIN_new_block(COTTON_WIN* cotton) {
  COTTON_WIN_BLOCK* block = calloc(1, sizeof(COTTON_WIN_BLOCK));
  
  return block;
}

LRESULT CALLBACK WndProc(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam) {
  
  static COTTON_WIN* cotton;
  
  static HWND block1;
  static HWND block2;
  
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
      
      cotton = (COTTON_WIN*)create_struct->lpCreateParams;
      
      {
        COTTON_WIN_NEW_BLOCK_WINDOW_ARGS new_block_args = {
          parent_window_handle : window_handle,
          window_id : 1,
        };
        block1 = COTTON_WIN_new_block_window(cotton, &new_block_args);
        MoveWindow(block1, 300, 200, 200, 45, 1);
      }
      
      {
        COTTON_WIN_NEW_BLOCK_WINDOW_ARGS new_block_args = {
          parent_window_handle : window_handle,
          window_id : 2,
        };
        block2 = COTTON_WIN_new_block_window(cotton, &new_block_args);
        MoveWindow(block2, 300, 250, 200, 45, 1);
      }

      return 0;
    }
    case WM_CTLCOLORBTN: {
      // デフォルトでコントールの背景は透過させる
      return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_DRAWITEM: {
      
      DRAWITEMSTRUCT* draw_item = (DRAWITEMSTRUCT*)lparam;
      HDC hdc = draw_item->hDC;
      HWND window_handle = draw_item->hwndItem;
      int32_t window_id = (int32_t)GetWindowLongPtr(window_handle, GWLP_ID);
      
      if (window_id == 2) {
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

        TCHAR buffer[200];
        wsprintf(buffer, TEXT("%s%d%s"), TEXT("ボタン"), window_id , TEXT("😀"));
        LPCTSTR button_text = buffer;
        SIZE text_size;
        GetTextExtentPoint32(hdc , button_text , lstrlen(button_text) , &text_size);
        
        {
          HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
          SelectObject(hdc, GetStockObject(NULL_PEN));
          
          HBRUSH brash;
          if (window_id == 1) {
            brash = CreateSolidBrush(RGB(0xee, 0x00, 0x00));
          }
          else {
            brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
          }
          SelectObject(hdc , brash);
          RoundRect(hdc , 0 , 0 , text_size.cx, text_size.cy, 10, 10);
          DeleteObject(SelectObject(hdc , GetStockObject(NULL_BRUSH)));
        }
        
        {
          
          TextOut(hdc , 0 , 0 , button_text , lstrlen(button_text));
          
          DeleteObject(hFont);
        }
      }

      break;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(window_handle , &ps);

      RECT client_rect;
      GetClientRect(window_handle , &client_rect);

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
        TextOut(hdc , 0 , 0 , TEXT("Cotton"), lstrlen(TEXT("Cotton")));
        DeleteObject(hFont);
      }

      {
        RECT main_window_rect;
    		GetWindowRect(window_handle, &main_window_rect);
        int cxSizeFrame = GetSystemMetrics(SM_CXSIZEFRAME); // 境界線幅X方向
        int cySizeFrame = GetSystemMetrics(SM_CYSIZEFRAME); // 境界線幅Y方向
        int cyCaption = GetSystemMetrics(SM_CYCAPTION);     // タイトルバーの高さ

        printf("AAAAAA %d %d %d %d\n", main_window_rect.left, main_window_rect.top, main_window_rect.right, main_window_rect.bottom);
        printf("BBBBBB %d %d %d\n", cxSizeFrame, cySizeFrame, cyCaption);
        
        int32_t abs_client_origin_left = main_window_rect.left + cxSizeFrame;
        int32_t abs_client_origin_top = main_window_rect.top + cySizeFrame + cyCaption;

        printf("CCCCCC %d %d\n", abs_client_origin_left, abs_client_origin_top);
        
        RECT block1_window_rect;
    		GetWindowRect(block1, &block1_window_rect);

        printf("DDDDDD %d %d %d %d\n", block1_window_rect.left, block1_window_rect.top, block1_window_rect.right, block1_window_rect.bottom);
        
        RECT block1_window_rect_rel;
        block1_window_rect_rel.left = block1_window_rect.left - abs_client_origin_left;
        block1_window_rect_rel.top = block1_window_rect.top - abs_client_origin_top;
        block1_window_rect_rel.right = block1_window_rect.right - abs_client_origin_left;
        block1_window_rect_rel.bottom = block1_window_rect.bottom - abs_client_origin_top;
        
        printf("%d %d %d %d\n", block1_window_rect_rel.left, block1_window_rect_rel.top, block1_window_rect_rel.right, block1_window_rect_rel.bottom);

        int32_t window_id = 1;

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

        TCHAR buffer[200];
        wsprintf(buffer, TEXT("%s%d%s"), TEXT("ボタン"), window_id , TEXT("😀"));
        LPCTSTR button_text = buffer;
        SIZE text_size;
        GetTextExtentPoint32(hdc , button_text , lstrlen(button_text) , &text_size);
        
        printf("EEEEEEEE %d %d", text_size.cx, text_size.cy);

        HBRUSH brash;
        HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0xee, 0x00, 0x00));
        if (window_id == 1) {
          brash = CreateSolidBrush(RGB(0xee, 0x00, 0x00));
        }
        else {
          brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
        }
        SelectObject(hdc , hpen);
        SelectObject(hdc , brash);
        RoundRect(hdc , block1_window_rect_rel.left, block1_window_rect_rel.top, block1_window_rect_rel.left + text_size.cx, block1_window_rect_rel.top + text_size.cy, 10, 10);
        DeleteObject(SelectObject(hdc , GetStockObject(NULL_BRUSH)));
        
        {
          HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
          SelectObject(hdc, GetStockObject(NULL_PEN));
          TextOut(hdc, block1_window_rect_rel.left, block1_window_rect_rel.top, button_text, lstrlen(button_text));
          DeleteObject(hpen);
        }
      }
      
      EndPaint(window_handle , &ps);
      return 0;
    }
  }
  return DefWindowProc(window_handle , message , wparam , lparam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine ,int nCmdShow ) {
  
  // Cotton Application for Windows
  COTTON_WIN* cotton = COTTON_WIN_new(NULL);
  cotton->dummy = 5;
  
  // Create main window
  COTTON_WIN_NEW_MAIN_WINDOW_ARGS new_main_window_args = {
    title : TEXT("Cotton"),
    cotton : cotton,
  };
  HWND main_window = COTTON_WIN_new_main_window(cotton, &new_main_window_args);
  if (main_window == NULL) return -1;
  
  // Get and dispatch message
  MSG message;
  while(GetMessage(&message , NULL , 0 , 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  
  return message.wParam;
}

HWND COTTON_WIN_new_main_window(COTTON_WIN* cotton, COTTON_WIN_NEW_MAIN_WINDOW_ARGS* args) {
  
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  
  // Register Window Class
  WNDCLASS winc;
  winc.style = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc = WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance = instance_handle;
  winc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName = TEXT("main_window");
  if (!RegisterClass(&winc)) return NULL;

  // Create Main Window
  const TCHAR* window_class_name = TEXT("main_window");
  const TCHAR* window_title = args->title;
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  int window_x = CW_USEDEFAULT;
  int window_y = CW_USEDEFAULT;
  int window_width = CW_USEDEFAULT;
  int window_heigth = CW_USEDEFAULT;
  HWND window_parent_window_handle = NULL;
  HMENU window_id = NULL;
  void* window_wm_create_lparam = (void*)args->cotton;
  HWND window_handle = CreateWindow(
      window_class_name, window_title,
      window_style,
      window_x, window_y,
      window_width, window_heigth,
      window_parent_window_handle, window_id, instance_handle, window_wm_create_lparam
  );

  return window_handle;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL);
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
