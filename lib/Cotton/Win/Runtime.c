#include <spvm_native.h>

#include <windows.h>

typedef struct cotton_win COTTON_WIN;
struct cotton_win {
  int32_t dummy;
};

typedef struct cotton_win_app_new_args COTTON_WIN_APP_NEW_ARGS;
struct cotton_win_app_new_args {
};

COTTON_WIN* COTTON_WIN_APP_new() {
  COTTON_WIN* cotton = calloc(1, sizeof(COTTON_WIN));
  
  return cotton;
}

COTTON_WIN COTTON_WIN_APP_free(COTTON_WIN* cotton) {
  free(cotton);
}

typedef struct cotton_win_app_new_main_window_args COTTON_WIN_APP_NEW_MAIN_WINDOW_ARGS;
struct cotton_win_app_new_main_window_args {
  LPCTSTR title;
  COTTON_WIN* cotton;
};

typedef struct cotton_win_app_new_node_window_args COTTON_WIN_APP_NEW_NODE_WINDOW_ARGS;
struct cotton_win_app_new_node_window_args {
  HWND parent_window_handle;
  int32_t window_id;
};

HWND COTTON_WIN_APP_new_main_window(COTTON_WIN* cotton, COTTON_WIN_APP_NEW_MAIN_WINDOW_ARGS* args);

enum {
  COTTON_WIN_APP_NODE_TYPE_ELEMENT,
  COTTON_WIN_APP_NODE_TYPE_TEXT,
};

enum {
  COTTON_WIN_APP_NODE_DISPLAY_BLOCK,
  COTTON_WIN_APP_NODE_DISPLAY_INLINE,
};

enum {
  COTTON_WIN_APP_NODE_TEXT_ALIGN_LEFT,
  COTTON_WIN_APP_NODE_TEXT_ALIGN_CENTER,
  COTTON_WIN_APP_NODE_TEXT_ALIGN_RIGHT,
};

typedef struct cotton_win_app_node COTTON_WIN_APP_NODE;
struct cotton_win_app_node {
  int8_t type;
  int32_t font_size;
  int32_t color;
  int8_t font_weight;
  const TCHAR* text;
  COTTON_WIN_APP_NODE* first;
  COTTON_WIN_APP_NODE* last;
  COTTON_WIN_APP_NODE* sibparent;
  int8_t moresib;
  int8_t display;
  int32_t padding_left;
  int32_t padding_top;
  int32_t padding_right;
  int32_t padding_bottom;
  int32_t text_align;
};

LRESULT CALLBACK COTTON_WIN_APP_WndProc(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam) {

  static COTTON_WIN* cotton;
  
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
      cotton = (COTTON_WIN*)create_struct->lpCreateParams;
      return 0;
    }
    case WM_PAINT: {
      // Get Device context
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(window_handle, &ps);

      COTTON_WIN_APP_NODE* elem_node1 = calloc(1, sizeof(COTTON_WIN_APP_NODE));
      
      elem_node1->type = COTTON_WIN_APP_NODE_TYPE_ELEMENT;
      elem_node1->padding_left = 5;
      elem_node1->padding_top = 5;
      elem_node1->padding_right = 5;
      elem_node1->padding_bottom = 5;
      elem_node1->text_align = COTTON_WIN_APP_NODE_TEXT_ALIGN_CENTER;

      COTTON_WIN_APP_NODE* text_node1 = calloc(1, sizeof(COTTON_WIN_APP_NODE));
      
      text_node1->type = COTTON_WIN_APP_NODE_TYPE_TEXT;
      text_node1->text = TEXT("あいうえおあああああああああああああああああああああああああああああああああああああああああああああああああああああああああああああああああああああ");
      
      elem_node1->first = text_node1;
      elem_node1->last = text_node1;
      text_node1->sibparent = elem_node1;

      
      // Render block which has text
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
        
        COTTON_WIN_APP_NODE* text_node = elem_node1->first;
        const TCHAR* text = text_node->text;

        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
        SetBkMode(hdc , TRANSPARENT);

        SIZE text_size;
        GetTextExtentPoint32(hdc, text, lstrlen(text), &text_size);

        RECT client_rect;
        GetClientRect(window_handle , &client_rect);
        int32_t width = elem_node1->padding_left + client_rect.right + elem_node1->padding_right;
        int32_t height = elem_node1->padding_top + text_size.cy + elem_node1->padding_bottom;
        
        UINT drow_text_flag = DT_WORDBREAK;
        if (elem_node1->text_align == COTTON_WIN_APP_NODE_TEXT_ALIGN_LEFT) {
          drow_text_flag |= DT_LEFT;
        }
        else if (elem_node1->text_align == COTTON_WIN_APP_NODE_TEXT_ALIGN_CENTER) {
          drow_text_flag |= DT_CENTER;
        }
        else if (elem_node1->text_align == COTTON_WIN_APP_NODE_TEXT_ALIGN_RIGHT) {
          drow_text_flag |= DT_RIGHT;
        }
        
        RECT text_rect = {right : width, bottom:50};
        DrawText(hdc, text, -1, &text_rect, drow_text_flag | DT_CALCRECT);
        
        // Draw block
        {
          HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
          SelectObject(hdc, hpen);
          HBRUSH brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
          SelectObject(hdc, brash);
          Rectangle(hdc, 0, 0, width, text_rect.bottom);
          DeleteObject(hpen);
          DeleteObject(brash);
        }
        
        // Draw text
        {
          DrawText(hdc, text, -1, &text_rect, drow_text_flag);
        }
        
        // Delete font handle
        DeleteObject(hFont);
      }
      
      EndPaint(window_handle , &ps);
      return 0;
    }
  }
  return DefWindowProc(window_handle , message , wparam , lparam);
}

HWND COTTON_WIN_APP_new_main_window(COTTON_WIN* cotton, COTTON_WIN_APP_NEW_MAIN_WINDOW_ARGS* args) {
  
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  
  // Register Window Class
  WNDCLASS winc;
  winc.style = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc = COTTON_WIN_APP_WndProc;
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

int32_t SPNATIVE__Cotton__Win__Runtime__run(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  void* sv_self = stack[0].oval;
  
  int32_t e;
  
  HINSTANCE hInst = GetModuleHandle(NULL);
  
  // Cotton Application for Windows
  COTTON_WIN* cotton = COTTON_WIN_APP_new(NULL);
  cotton->dummy = 5;
  
  void* sv_title = env->get_field_object_by_name(env, sv_self, "Cotton::Win::Runtime", "title", "string", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  void* sv_title_u16 = NULL;
  {
    stack[0].oval = sv_title;
    e = env->call_sub_by_name(env, "SPVM::Unicode", "utf8_to_utf16", "short[](string)", stack, __FILE__, __LINE__);
    if (e) { return e; }
    sv_title_u16 = stack[0].oval;
  }

  // Create main window
  COTTON_WIN_APP_NEW_MAIN_WINDOW_ARGS new_main_window_args = {
    title : env->get_elems_short(env, sv_title_u16),
    cotton : cotton,
  };
  HWND main_window = COTTON_WIN_APP_new_main_window(cotton, &new_main_window_args);
  if (main_window == NULL) return -1;
  
  // Get and dispatch message
  MSG message;
  while(GetMessage(&message , NULL , 0 , 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return 0;
}