#include <spvm_native.h>

#include <windows.h>

int16_t* COTTON_WIN_RUNTIME_utf8_to_utf16(SPVM_ENV* env, const char* string) {
  int32_t e;
  SPVM_VALUE stack[256];
  
  void* sv_string = env->new_string_nolen(env, string);
  
  void* sv_string_u16 = NULL;
  {
    stack[0].oval = sv_string;
    e = env->call_sub_by_name(env, "SPVM::Unicode", "utf8_to_utf16", "short[](string)", stack, __FILE__, __LINE__);
    if (e) { return NULL; }
    sv_string_u16 = stack[0].oval;
  }
  
  int16_t* string_u16 = env->get_elems_short(env, sv_string_u16);
  
  return string_u16;
}

void COTTON_WIN_RUNTIME_alert(SPVM_ENV* env, const char* message) {
  int16_t* message_u16 = COTTON_WIN_RUNTIME_utf8_to_utf16(env, message);
  
  MessageBoxW(NULL, message_u16, TEXT("Alert"), MB_OK);
}

static void alert(SPVM_ENV* env, const char* message) {
  COTTON_WIN_RUNTIME_alert(env, message);
}

struct COTTON_RUNTIME_PAINT_INFO {
  HDC hdc;
};

HWND COTTON_WIN_RUNTIME_new_main_window(SPVM_ENV* env, void* sv_app);

int32_t Cotton_Runtime_paint(SPVM_ENV* env, void* sv_app, HWND window_handle) {
  SPVM_VALUE stack[256];
  int32_t e = 0;
  
  // Draw page title
  {
    void* sv_app_name = NULL;
    {
      stack[0].oval = sv_app;
      e = env->call_sub_by_name(env, "Cotton::App", "name", "string(self)", stack, __FILE__, __LINE__);
      if (e) { return e; }
      sv_app_name = stack[0].oval;
    }
    
    void* sv_app_name_u16 = NULL;
    {
      stack[0].oval = sv_app_name;
      e = env->call_sub_by_name(env, "SPVM::Unicode", "utf8_to_utf16", "short[](string)", stack, __FILE__, __LINE__);
      if (e) { return e; }
      sv_app_name_u16 = stack[0].oval;
    }
    int16_t* app_name_u16 = env->get_elems_short(env, sv_app_name_u16);
    
    SetWindowTextW(window_handle, app_name_u16);
  }
  
  // Draw parent area
  {
    // Begin paint and get Device context
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(window_handle, &ps);

    // Call Cotton::App->paint_nodes
    {
      int32_t scope = env->enter_scope(env);
      
      struct COTTON_RUNTIME_PAINT_INFO* paint_info = calloc(1, sizeof(struct COTTON_RUNTIME_PAINT_INFO));
      paint_info->hdc = hdc;
      
      void* sv_paint_info = env->new_pointer_by_name(env, "Cotton::PaintInfo", paint_info, &e, __FILE__, __LINE__);
      if (e) { return e; }

      stack[0].oval = sv_app;
      stack[1].oval = sv_paint_info;
      e = env->call_sub_by_name(env, "Cotton::App", "paint_nodes", "void(self,Cotton::PaintInfo)", stack, __FILE__, __LINE__);
      if (e) { return e; }
      
      free(paint_info);
      env->leave_scope(env, scope);
    }
    
    // End paint
    EndPaint(window_handle , &ps);
  }
  
  return 0;
}

LRESULT CALLBACK COTTON_WIN_RUNTIME_WndProc(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam) {
  
  static SPVM_ENV* env;
  static void* sv_app;
  
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
      void** wm_create_args = (void**)create_struct->lpCreateParams;
      env = wm_create_args[0];
      sv_app = (void*)wm_create_args[1];

      // COTTON_WIN_RUNTIME_alert(env, "ハローワールド");

      return 0;
    }
    case WM_PAINT: {
      int32_t e = 0;
      
      // Draw node
      e = Cotton_Runtime_paint(env, sv_app, window_handle);
      
      if (e) {
        alert(env, env->get_chars(env, env->get_exception(env)));
        PostQuitMessage(0);
        return 0;
      }
      
      return 0;
    }
  }
  return DefWindowProc(window_handle , message , wparam , lparam);
}

HWND COTTON_WIN_RUNTIME_new_main_window(SPVM_ENV* env, void* sv_app) {
  
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  
  // Register Window Class
  WNDCLASS winc;
  winc.style = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc = COTTON_WIN_RUNTIME_WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance = instance_handle;
  winc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName = TEXT("main_window");
  if (!RegisterClass(&winc)) return NULL;

  // Create Main Window
  const int16_t* window_class_name = TEXT("main_window");
  const int16_t* window_title = NULL;
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  int window_x = CW_USEDEFAULT;
  int window_y = CW_USEDEFAULT;
  int window_width = CW_USEDEFAULT;
  int window_heigth = CW_USEDEFAULT;
  HWND window_parent_window_handle = NULL;
  HMENU window_id = NULL;
  void** wm_create_args = calloc(2, sizeof(void*));
  wm_create_args[0] = env;
  wm_create_args[1] = sv_app;
  void* window_wm_create_lparam = (void*)wm_create_args;
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
  
  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Win::Runtime", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }

  // Create main window
  HWND main_window = COTTON_WIN_RUNTIME_new_main_window(env, sv_app);
  if (main_window == NULL) return -1;
  
  // Get and dispatch message
  MSG message;
  while(GetMessage(&message , NULL , 0 , 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  
  return 0;
}

int32_t SPNATIVE__Cotton__Win__Runtime__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;
  void* sv_paint_info = stack[1].oval;
  void* sv_node = stack[2].oval;
  
  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, sv_paint_info);
  HDC hdc = paint_info->hdc;

  int32_t draw_left = env->get_field_int_by_name(env, sv_node, "Cotton::Node", "draw_left", &e, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_top = env->get_field_int_by_name(env, sv_node, "Cotton::Node", "draw_top", &e, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_width = env->get_field_int_by_name(env, sv_node, "Cotton::Node", "draw_width", &e, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_height = env->get_field_int_by_name(env, sv_node, "Cotton::Node", "draw_height", &e, __FILE__, __LINE__);
  if (e) { return e; }

  // Draw block
  {
    HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
    SelectObject(hdc, hpen);
    HBRUSH brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
    SelectObject(hdc, brash);
    Rectangle(hdc, draw_left, draw_top, draw_width - 1, draw_height - 1);
    DeleteObject(hpen);
    DeleteObject(brash);
  }
  
  void* sv_text = env->get_field_object_by_name(env, sv_node, "Cotton::Node", "text", "string", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  if (sv_text) {
    // Render block which has text
    const char* text = env->get_chars(env, sv_text);
    
    const int16_t* text_utf16 = COTTON_WIN_RUNTIME_utf8_to_utf16(env, text);

    RECT parent_rect = {left : 0, top : 0, bottom: 100, right: 800};
    // GetClientRect(window_handle , &parent_rect);

    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = parent_rect.right + 1;
    int32_t parent_height = parent_rect.bottom + 1;
    
    int32_t color = RGB(0xFF, 0x00, 0x00);
    int32_t background_color = RGB(0x00, 0xAA, 0x77);
    
    // draw width
    int32_t draw_width = parent_width;
    
    // Font
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
    UINT drow_text_flag = DT_WORDBREAK;
    drow_text_flag |= DT_LEFT;
    
    // Culcurate text height
    RECT culc_node_rect = {top : 200, right : draw_width - 1};
    DrawText(hdc, text_utf16, -1, &culc_node_rect, drow_text_flag | DT_CALCRECT);
    
    // draw height
    int32_t draw_height = culc_node_rect.bottom + 1;

    // Draw text
    {
      SetTextColor(hdc, color);
      SetBkMode(hdc , TRANSPARENT);
      DrawText(hdc, text_utf16, -1, &culc_node_rect, drow_text_flag);
    }
    
    // Delete font handle
    DeleteObject(hFont);
  }


  return 0;
}
