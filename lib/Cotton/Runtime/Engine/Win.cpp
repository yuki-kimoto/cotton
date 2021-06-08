#include <spvm_native.h>

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <assert.h>

#include <iostream>

extern "C" {
int16_t* COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(SPVM_ENV* env, const char* string) {
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

void COTTON_RUNTIME_ENGINE_WIN_alert(SPVM_ENV* env, const char* message) {
  int16_t* message_u16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, message);
  
  MessageBoxW(NULL, (LPCWSTR)message_u16, TEXT("Alert"), MB_OK);
}

static void alert(SPVM_ENV* env, const char* message) {
  COTTON_RUNTIME_ENGINE_WIN_alert(env, message);
}

struct COTTON_RUNTIME_PAINT_INFO {
  HDC hdc;
  HWND window_handle;
  ID2D1HwndRenderTarget* renderer;
};

int32_t Cotton_Runtime_paint_window(SPVM_ENV* env, void* sv_self) {
  SPVM_VALUE stack[256];
  int32_t e = 0;
  
  void* sv_window_handle = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "window_handle", "Cotton::Runtime::Engine::Win::WindowHandle", &e, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_runtime = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "runtime", "Cotton::Runtime", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, sv_window_handle);
  
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
    
    SetWindowTextW(window_handle, (LPCWSTR)app_name_u16);
  }
  
  // Draw parent area
  {
    // Begin paint and get Device context
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(window_handle, &ps);

    // Result for COM. Direct 2D is COM.
    HRESULT com_result = S_OK;
    
    // Create Direct2D factory
    ID2D1Factory* renderer_factory = NULL;
    com_result = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &renderer_factory);
    if (FAILED(com_result)) {
      fprintf(stderr, "Fail D2D1CreateFactory\n");
      return 1;
    }

    // Viewport rect
    RECT viewport_rect;
    GetClientRect(window_handle, &viewport_rect);
    
    // Viewport size
    D2D1_SIZE_U viewport_size = {(UINT32)(viewport_rect.right + 1), (UINT32)(viewport_rect.bottom + 1)};
    
    // Renderer
    ID2D1HwndRenderTarget* renderer = NULL;
    com_result = renderer_factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(window_handle, viewport_size),
      &renderer
    );
    if (FAILED(com_result) ) {
      fprintf(stderr, "Fail CreateHwndRenderTarget\n");
      return 1;
    }
        
    // Start Draw
    renderer->BeginDraw();

    // Clear viewport
    D2D1_COLOR_F viewport_init_background_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    renderer->Clear(viewport_init_background_color);

    // Call Cotton::Runtime->paint_nodes
    {
      int32_t scope = env->enter_scope(env);
      
      struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)calloc(1, sizeof(struct COTTON_RUNTIME_PAINT_INFO));
      paint_info->hdc = hdc;
      paint_info->window_handle = window_handle;
      paint_info->renderer = renderer;
      
      void* sv_paint_info = env->new_pointer_by_name(env, "Cotton::PaintInfo", paint_info, &e, __FILE__, __LINE__);
      if (e) { return e; }
      
      stack[0].oval = sv_runtime;
      stack[1].oval = sv_paint_info;
      e = env->call_sub_by_name(env, "Cotton::Runtime", "paint_nodes", "void(self,Cotton::PaintInfo)", stack, __FILE__, __LINE__);
      if (e) { return e; }


      free(paint_info);
      env->leave_scope(env, scope);
    }
    
    // End draw
    renderer->EndDraw();
            
    // End paint
    EndPaint(window_handle , &ps);
  }
  
  return 0;
}

LRESULT CALLBACK COTTON_RUNTIME_ENGINE_WIN_WndProc(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam) {
  
  static SPVM_ENV* env;
  static void* sv_self;
  
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
      void** wm_create_args = (void**)create_struct->lpCreateParams;
      env = (SPVM_ENV*)wm_create_args[0];
      sv_self = (void*)wm_create_args[1];

      // COTTON_RUNTIME_ENGINE_WIN_alert(env, "ハローワールド");

      return 0;
    }
    case WM_PAINT: {
      int32_t e = 0;
      
      // Draw node
      e = Cotton_Runtime_paint_window(env, sv_self);
      
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

int32_t SPNATIVE__Cotton__Runtime__Engine__Win__run(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  // Get and dispatch message
  MSG message;
  while(GetMessage(&message , NULL , 0 , 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  
  return 0;
}

int32_t SPNATIVE__Cotton__Runtime__Engine__Win__create_main_window(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  void* sv_self = stack[0].oval;
  
  int32_t e = 0;
  
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  
  // Register Window Class
  WNDCLASS winc;
  winc.style = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc = COTTON_RUNTIME_ENGINE_WIN_WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance = instance_handle;
  winc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName = TEXT("main_window");
  if (!RegisterClass(&winc)) { return env->die(env, "Can't register window class"); };

  // Create Main Window
  const int16_t* window_class_name = (const int16_t*)TEXT("main_window");
  const int16_t* window_title = NULL;
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  int window_x = CW_USEDEFAULT;
  int window_y = CW_USEDEFAULT;
  int window_width = CW_USEDEFAULT;
  int window_heigth = CW_USEDEFAULT;
  HWND window_parent_window_handle = NULL;
  HMENU window_id = NULL;
  void** wm_create_args = (void**)calloc(2, sizeof(void*));
  wm_create_args[0] = env;
  wm_create_args[1] = sv_self;
  void* window_wm_create_lparam = (void*)wm_create_args;
  HWND window_handle = CreateWindow(
      (LPCWSTR)window_class_name, (LPCWSTR)window_title,
      window_style,
      window_x, window_y,
      window_width, window_heigth,
      window_parent_window_handle, window_id, instance_handle, window_wm_create_lparam
  );
  
  void* sv_window_handle = env->new_pointer_by_name(env, "Cotton::Runtime::Engine::Win::WindowHandle", window_handle, &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  env->set_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "window_handle", "Cotton::Runtime::Engine::Win::WindowHandle", sv_window_handle, &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  return 0;
}

int32_t SPNATIVE__Cotton__Runtime__Engine__Win__calc_text_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;
  void* sv_paint_info = stack[1].oval;
  void* sv_text = stack[2].oval;
  int32_t draw_width = stack[3].ival;
  void* sv_font_styles = stack[4].oval;

  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, sv_paint_info);
  HDC hdc = paint_info->hdc;
  
  int32_t draw_height = 0;
  if (sv_text) {
    // Render block which has text
    const char* text = env->get_chars(env, sv_text);
    
    const int16_t* text_utf16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;

    RECT parent_rect = {.left = 0, .top = 0, .right = draw_width - 1};

    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = parent_rect.right + 1;
    int32_t parent_height = parent_rect.bottom + 1;
    
    int32_t color = RGB(0xFF, 0x00, 0x00);
    int32_t background_color = RGB(0x00, 0xAA, 0x77);
    
    // draw width
    int32_t draw_width = parent_width;
    
    // Culcurate text height
    RECT culc_node_rect = {.left = parent_rect.left, .top = parent_rect.top, .right = parent_rect.right, .bottom = parent_rect.bottom};
    
    HRESULT com_result;
    
    IDWriteFactory* direct_write_factory = NULL;
    com_result = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>( &direct_write_factory ) );

    // Create text format
    IDWriteTextFormat* text_format = NULL;
    direct_write_factory->CreateTextFormat(
      L"Meiryo",
      NULL,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      40,
      L"",
      &text_format
    );
    
    // 
    IDWriteTextLayout* text_layout = NULL;
    com_result = direct_write_factory->CreateTextLayout(
      (const WCHAR*)text_utf16,
      text_utf16_length,
      text_format,
      draw_width,
      0,
      &text_layout
    );
    
    // Get text metrics
    DWRITE_TEXT_METRICS text_metrics;
    text_layout->GetMetrics( &text_metrics );
    
    draw_height = text_metrics.height;
  }
  
  stack[0].ival = draw_height;
  
  return 0;
}

int32_t SPNATIVE__Cotton__Runtime__Engine__Win__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;
  void* sv_paint_info = stack[1].oval;
  void* sv_node = stack[2].oval;

  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }

  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, sv_paint_info);
  HDC hdc = paint_info->hdc;
  ID2D1HwndRenderTarget* renderer = paint_info->renderer;

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
    void* sv_background_color = env->get_field_object_by_name(env, sv_node, "Cotton::Node", "background_color", "Cotton::Color", &e, __FILE__, __LINE__);
    if (e) { return e; }
    
    int32_t background_color;
    D2D1::ColorF background_color_f = D2D1::ColorF(0, 0, 1.0f, 1.0f);
    if (sv_background_color) {
      float background_color_red = env->get_field_float_by_name(env, sv_background_color, "Cotton::Color", "red", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_green = env->get_field_float_by_name(env, sv_background_color, "Cotton::Color", "green", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_blue = env->get_field_float_by_name(env, sv_background_color, "Cotton::Color", "blue", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_alpha = env->get_field_float_by_name(env, sv_background_color, "Cotton::Color", "alpha", &e, __FILE__, __LINE__);
      if (e) { return e; }

      background_color_f = D2D1::ColorF(background_color_red, background_color_green, background_color_blue, background_color_alpha);
    }
    else {
      background_color_f = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0);
    }

    // Create background brash
    ID2D1SolidColorBrush* background_brush = NULL;
    renderer->CreateSolidColorBrush(
      background_color_f,
      &background_brush
    );
    assert(background_brush);

    // 描画矩形
    D2D1_RECT_F block_rect = D2D1::RectF(draw_left, draw_top, draw_width - 1, draw_height - 1);

    // 四角形の描画
    renderer->FillRectangle(&block_rect, background_brush);
    
    // ブラシの破棄
    background_brush->Release();
  }
  
  void* sv_text = env->get_field_object_by_name(env, sv_node, "Cotton::Node", "text", "string", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  if (sv_text) {
    // Render block which has text
    const char* text = env->get_chars(env, sv_text);
    
    const int16_t* text_utf16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;

    RECT parent_rect = {.left = draw_left, .top = draw_top, .right = draw_width - 1, .bottom = draw_height - 1};

    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = parent_rect.right + 1;
    int32_t parent_height = parent_rect.bottom + 1;

    void* sv_color = env->get_field_object_by_name(env, sv_node, "Cotton::Node", "color", "Cotton::Color", &e, __FILE__, __LINE__);
    if (e) { return e; }
    
    int32_t color;
    D2D1::ColorF color_f = D2D1::ColorF(0, 0, 1.0f, 1.0f);
    if (sv_color) {
      float color_red = env->get_field_float_by_name(env, sv_color, "Cotton::Color", "red", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float color_green = env->get_field_float_by_name(env, sv_color, "Cotton::Color", "green", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float color_blue = env->get_field_float_by_name(env, sv_color, "Cotton::Color", "blue", &e, __FILE__, __LINE__);
      if (e) { return e; }

      float color_alpha = env->get_field_float_by_name(env, sv_color, "Cotton::Color", "alpha", &e, __FILE__, __LINE__);
      if (e) { return e; }

      color = RGB(color_red, color_green, color_blue);
      color_f = D2D1::ColorF(color_red, color_green, color_blue, color_alpha);
    }
    else {
      color = RGB(0xFF, 0x00, 0x00);
      color_f = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    // draw width
    int32_t draw_width = parent_width;
    
    // COM result
    HRESULT com_result;
    
    // DirectWrite factory
    IDWriteFactory* direct_write_factory = NULL;
    com_result = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>( &direct_write_factory ) );

    // Create text format
    IDWriteTextFormat* text_format = NULL;
    direct_write_factory->CreateTextFormat(
      L"Meiryo",
      NULL,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      40,
      L"",
      &text_format
    );
    
    // Create text layout
    IDWriteTextLayout* text_layout = NULL;
    com_result = direct_write_factory->CreateTextLayout(
          (const WCHAR*)text_utf16       // 文字列
        , text_utf16_length        // 文字列の幅
        ,text_format           // DWriteTextFormat
        , draw_width    // 枠の幅
        , 0    // 枠の高さ
        , &text_layout
    );

    // Create text brush
    ID2D1SolidColorBrush* text_brush = NULL;
    renderer->CreateSolidColorBrush(
      color_f,
      &text_brush
    );
    
    // Draw text
    D2D1_POINT_2F point = {.x = (float)parent_rect.left, .y = (float)parent_rect.top};
    renderer->DrawTextLayout(point, text_layout, text_brush);
  }

  return 0;
}

int32_t SPNATIVE__Cotton__Runtime__Engine__Win__get_viewport_width(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;

  void* sv_window_handle = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "window_handle", "Cotton::Runtime::Engine::Win::WindowHandle", &e, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, sv_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);

  stack[0].ival = rect.right + 1;

  return 0;
}


int32_t SPNATIVE__Cotton__Runtime__Engine__Win__get_viewport_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;

  void* sv_self = stack[0].oval;

  void* sv_window_handle = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "window_handle", "Cotton::Runtime::Engine::Win::WindowHandle", &e, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Runtime::Engine::Win", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, sv_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);

  stack[0].ival = rect.bottom + 1;
  
  return 0;
}
}
