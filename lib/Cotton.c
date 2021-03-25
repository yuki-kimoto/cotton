#include <spvm_native.h>

#include <windows.h>

typedef struct cotton COTTON_WIN;
struct cotton_win {
  HINSTANCE instance_handle;
};

typedef struct cotton_win_new_main_window_args COTTON_WIN_NEW_MAIN_WINDOW_ARGS;
struct cotton_win_new_main_window_args {
  LPCTSTR title;
};

typedef struct cotton_win_new_block_args COTTON_WIN_NEW_BLOCK_ARGS;
struct cotton_win_new_block_args {
  HWND parent_window_handle;
  int32_t window_id;
};

HWND COTTON_WIN_new_main_window(COTTON_WIN_NEW_MAIN_WINDOW_ARGS* args);
HWND COTTON_WIN_new_block(COTTON_WIN_NEW_BLOCK_ARGS* args);

HWND COTTON_WIN_new_block(COTTON_WIN_NEW_BLOCK_ARGS* args) {

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

LRESULT CALLBACK WndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp) {
  
  static HWND block1 = NULL;
  static HWND block2 = NULL;
  
  switch (msg) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      {
        COTTON_WIN_NEW_BLOCK_ARGS new_block_args = {
          parent_window_handle : hwnd,
          window_id : 1,
        };
        block1 = COTTON_WIN_new_block(&new_block_args);
        MoveWindow(block1, 300, 200, 200, 45, 1);
      }
      
      {
        COTTON_WIN_NEW_BLOCK_ARGS new_block_args = {
          parent_window_handle : hwnd,
          window_id : 2,
        };
        block2 = COTTON_WIN_new_block(&new_block_args);
        MoveWindow(block2, 300, 250, 200, 45, 1);
      }

      return 0;
    }
    case WM_CTLCOLORBTN: {
      
      return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    case WM_DRAWITEM: {
      
      LPDRAWITEMSTRUCT draw_item = (LPDRAWITEMSTRUCT)lp;
      HDC hdc = draw_item->hDC;
      HWND hwnd = draw_item->hwndItem;
      int32_t window_id = (int32_t)GetWindowLongPtr(hwnd, GWLP_ID);

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

      break;
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
      
      EndPaint(hwnd , &ps);
      return 0;
    }
  }
  return DefWindowProc(hwnd , msg , wp , lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine ,int nCmdShow ) {
  
  // Create main window
  COTTON_WIN_NEW_MAIN_WINDOW_ARGS new_main_window_args = {
    title : TEXT("Cotton")
  };
  HWND main_window = COTTON_WIN_new_main_window(&new_main_window_args);
  if (main_window == NULL) return -1;
  
  // Get and dispatch message
  MSG msg;
  while(GetMessage(&msg , NULL , 0 , 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  return msg.wParam;
}

HWND COTTON_WIN_new_main_window(COTTON_WIN_NEW_MAIN_WINDOW_ARGS* args) {
  
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
  LPCTSTR window_class_name = TEXT("main_window");
  LPCTSTR window_title = args->title;
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  int window_x = CW_USEDEFAULT;
  int window_y = CW_USEDEFAULT;
  int window_width = CW_USEDEFAULT;
  int window_heigth = CW_USEDEFAULT;
  HWND window_parent_window_handle = NULL;
  HMENU window_id = NULL;
  LPVOID window_create_lparam = NULL;
  HWND hwnd = CreateWindow(
      window_class_name, window_title,
      window_style,
      window_x, window_y,
      window_width, window_heigth,
      window_parent_window_handle, window_id, instance_handle, window_create_lparam
  );

  return hwnd;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL);
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
