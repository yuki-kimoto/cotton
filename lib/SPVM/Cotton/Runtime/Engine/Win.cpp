#include <spvm_native.h>

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <assert.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <iostream>

extern "C" {
int16_t* COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(SPVM_ENV* env, SPVM_VALUE* stack, const char* string) {
  int32_t e;
  
  void* sv_string =  env->new_string_nolen(env, stack, string);
  
  void* sv_string_utf8_to_utf16 = NULL;
  {
    stack[0].oval = sv_string;
    e = env->call_class_method_by_name(env, stack, "Unicode", "utf8_to_utf16", 1, __func__, __FILE__, __LINE__);
    if (e) { return NULL; }
    sv_string_utf8_to_utf16 = stack[0].oval;
  }
  
  int16_t* string_utf8_to_utf16 = env->get_elems_short(env, stack, sv_string_utf8_to_utf16);
  
  return string_utf8_to_utf16;
}

void COTTON_RUNTIME_ENGINE_WIN_alert(SPVM_ENV* env, SPVM_VALUE* stack, const char* message) {
  int16_t* message_utf8_to_utf16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, stack, message);
  
  MessageBoxW(NULL, (LPCWSTR)message_utf8_to_utf16, TEXT("Alert"), MB_OK);
}

static void alert(SPVM_ENV* env, SPVM_VALUE* stack, const char* message) {
  COTTON_RUNTIME_ENGINE_WIN_alert(env, stack, message);
}

struct COTTON_RUNTIME_PAINT_INFO {
  HDC hdc;
  HWND window_handle;
  ID2D1HwndRenderTarget* renderer;
};

int32_t Cotton_Runtime_paint_window(SPVM_ENV* env, SPVM_VALUE* stack, void* sv_self) {
  int32_t e = 0;
  
  void* sv_window_handle = env->get_field_object_by_name(env, stack, sv_self, "window_handle", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, stack, sv_self, "app", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_runtime = env->get_field_object_by_name(env, stack, sv_self, "runtime", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, sv_window_handle);
  
  // Draw page title
  {
    void* sv_app_name = NULL;
    {
      stack[0].oval = sv_app;
      e = env->call_instance_method_by_name(env, stack, "name", 1, __func__, __FILE__, __LINE__);
      if (e) { return e; }
      sv_app_name = stack[0].oval;
    }
    
    void* sv_app_name_utf8_to_utf16 = NULL;
    {
      stack[0].oval = sv_app_name;
      e = env->call_class_method_by_name(env, stack, "Unicode", "utf8_to_utf16", 1, __func__, __FILE__, __LINE__);
      if (e) { return e; }
      sv_app_name_utf8_to_utf16 = stack[0].oval;
    }
    int16_t* app_name_utf8_to_utf16 = env->get_elems_short(env, stack, sv_app_name_utf8_to_utf16);
    
    SetWindowTextW(window_handle, (LPCWSTR)app_name_utf8_to_utf16);
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
      int32_t scope = env->enter_scope(env, stack);
      
      struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)calloc(1, sizeof(struct COTTON_RUNTIME_PAINT_INFO));
      paint_info->hdc = hdc;
      paint_info->window_handle = window_handle;
      paint_info->renderer = renderer;
      
      void* sv_paint_info = env->new_pointer_by_name(env, stack, "Cotton::PaintInfo", paint_info, &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }
      
      stack[0].oval = sv_runtime;
      stack[1].oval = sv_paint_info;
      e = env->call_instance_method_by_name(env, stack, "paint_nodes", 2, __func__, __FILE__, __LINE__);
      if (e) { return e; }


      free(paint_info);
      env->leave_scope(env, stack, scope);
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
  static SPVM_VALUE* stack;
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
      stack = (SPVM_VALUE*)wm_create_args[2];

      // COTTON_RUNTIME_ENGINE_WIN_alert(env, stack, "ハローワールド");

      return 0;
    }
    case WM_PAINT: {
      int32_t e = 0;
      
      // Draw node
      e = Cotton_Runtime_paint_window(env, stack, sv_self);
      
      if (e) {
        alert(env, stack, env->get_chars(env, stack, env->get_exception(env, stack)));
        PostQuitMessage(0);
        return 0;
      }
      
      return 0;
    }
  }
  return DefWindowProc(window_handle , message , wparam , lparam);
}

int32_t SPVM__Cotton__Runtime__Engine__Win__run(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  // Get and dispatch message
  MSG message;
  while (1) {
    if (PeekMessage(&message , NULL , 0 , 0, PM_NOREMOVE)) {
      if (GetMessage(&message , NULL , 0 , 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
      else {
        break;
      }
    }
  }
  
  return 0;
}

int32_t SPVM__Cotton__Runtime__Engine__Win__calc_text_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;
  void* sv_paint_info = stack[1].oval;
  void* sv_text = stack[2].oval;
  int32_t draw_width = stack[3].ival;
  void* sv_font_styles = stack[4].oval;

  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, stack, sv_paint_info);
  HDC hdc = paint_info->hdc;
  
  int32_t draw_height = 0;
  if (sv_text) {
    // Render block which has text
    const char* text = env->get_chars(env, stack, sv_text);
    
    const int16_t* text_utf16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, stack, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;

    RECT parent_rect = {.left = 0, .top = 0, .right = draw_width};

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

int32_t SPVM__Cotton__Runtime__Engine__Win__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;
  void* sv_paint_info = stack[1].oval;
  void* sv_node = stack[2].oval;

  void* sv_app = env->get_field_object_by_name(env, stack, sv_self, "app", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, stack, sv_paint_info);
  HDC hdc = paint_info->hdc;
  ID2D1HwndRenderTarget* renderer = paint_info->renderer;

  int32_t draw_left = env->get_field_int_by_name(env, stack, sv_node, "draw_left", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_top = env->get_field_int_by_name(env, stack, sv_node, "draw_top", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_width = env->get_field_int_by_name(env, stack, sv_node, "draw_width", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  int32_t draw_height = env->get_field_int_by_name(env, stack, sv_node, "draw_height", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  // Block rect
  D2D1_RECT_F block_rect = D2D1::RectF(draw_left, draw_top, draw_width, draw_height);
  
  printf("EEEEE %f %f\n", block_rect.left, block_rect.right);

  // Draw block
  {
    void* sv_background_color = env->get_field_object_by_name(env, stack, sv_node, "background_color", &e, __func__, __FILE__, __LINE__);
    if (e) { return e; }
    
    int32_t background_color;
    D2D1::ColorF background_color_f = {0};
    if (sv_background_color) {
      float background_color_red = env->get_field_float_by_name(env, stack,  sv_background_color, "red", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_green = env->get_field_float_by_name(env, stack,  sv_background_color, "green", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_blue = env->get_field_float_by_name(env, stack,  sv_background_color, "blue", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float background_color_alpha = env->get_field_float_by_name(env, stack,  sv_background_color, "alpha", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      background_color_f = D2D1::ColorF(background_color_red, background_color_green, background_color_blue, background_color_alpha);
    }
    else {
      printf("GGGGGGGGGG\n");
      background_color_f = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0);
    }

    // Create background brash
    ID2D1SolidColorBrush* background_brush = NULL;
    renderer->CreateSolidColorBrush(
      background_color_f,
      &background_brush
    );
    assert(background_brush);
    
    printf("FFFFFFFFFF %d %d %d %d\n", draw_left, draw_top, draw_width, draw_height);
    
    // 四角形の描画
    renderer->FillRectangle(&block_rect, background_brush);
    
    // ブラシの破棄
    background_brush->Release();
  }
  
  void* sv_text = env->get_field_object_by_name(env, stack, sv_node, "text", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  if (sv_text) {
    // Render block which has text
    const char* text = env->get_chars(env, stack, sv_text);
    
    const int16_t* text_utf16 = COTTON_RUNTIME_ENGINE_WIN_utf8_to_utf16(env, stack, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;

    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = block_rect.right + 1;
    int32_t parent_height = block_rect.bottom + 1;

    void* sv_color = env->get_field_object_by_name(env, stack, sv_node, "color", &e, __func__, __FILE__, __LINE__);
    if (e) { return e; }
    
    int32_t color;
    D2D1::ColorF color_f = D2D1::ColorF(0, 0, 1.0f, 1.0f);
    if (sv_color) {
      float color_red = env->get_field_float_by_name(env, stack,  sv_color, "red", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float color_green = env->get_field_float_by_name(env, stack,  sv_color, "green", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float color_blue = env->get_field_float_by_name(env, stack,  sv_color, "blue", &e, __func__, __FILE__, __LINE__);
      if (e) { return e; }

      float color_alpha = env->get_field_float_by_name(env, stack,  sv_color, "alpha", &e, __func__, __FILE__, __LINE__);
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
    D2D1_POINT_2F point = {.x = (float)block_rect.left, .y = (float)block_rect.top};
    renderer->DrawTextLayout(point, text_layout, text_brush);
  }

  return 0;
}

int32_t SPVM__Cotton__Runtime__Engine__Win__get_viewport_width(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;
  
  void* sv_self = stack[0].oval;

  void* sv_window_handle = env->get_field_object_by_name(env, stack, sv_self, "window_handle", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, stack, sv_self, "app", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, sv_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);

  stack[0].ival = rect.right + 1;
  
  printf("DDDDDDDD %d\n", stack[0].ival);

  return 0;
}


int32_t SPVM__Cotton__Runtime__Engine__Win__get_viewport_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t e;

  void* sv_self = stack[0].oval;

  void* sv_window_handle = env->get_field_object_by_name(env, stack, sv_self, "window_handle", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }

  void* sv_app = env->get_field_object_by_name(env, stack, sv_self, "app", &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, sv_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);

  stack[0].ival = rect.bottom + 1;
  
  return 0;
}

struct Vertex {
        float pos[ 3 ];
        float col[ 4 ];
};
 
int32_t SPVM__Cotton__Runtime__Engine__Win__create_main_window(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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
  if (!RegisterClass(&winc)) { return env->die(env, stack, "Can't register window class"); };

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
  void** wm_create_args = (void**)calloc(3, sizeof(void*));
  wm_create_args[0] = env;
  wm_create_args[1] = sv_self;
  wm_create_args[2] = stack;
  void* window_wm_create_lparam = (void*)wm_create_args;
  HWND window_handle = CreateWindow(
      (LPCWSTR)window_class_name, (LPCWSTR)window_title,
      window_style,
      window_x, window_y,
      window_width, window_heigth,
      window_parent_window_handle, window_id, instance_handle, window_wm_create_lparam
  );
  
  void* sv_window_handle = env->new_pointer_by_name(env, stack, "Cotton::Runtime::Engine::Win::WindowHandle", window_handle, &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  env->set_field_object_by_name(env, stack, sv_self, "window_handle", sv_window_handle, &e, __func__, __FILE__, __LINE__);
  if (e) { return e; }
  
  {
    {
      // Swap chain descriptor
      DXGI_SWAP_CHAIN_DESC sd;
      ZeroMemory( &sd, sizeof( sd ) );
      sd.BufferCount = 1;
      sd.BufferDesc.Width = 640;
      sd.BufferDesc.Height = 480;
      sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      sd.BufferDesc.RefreshRate.Numerator = 60;
      sd.BufferDesc.RefreshRate.Denominator = 1;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.OutputWindow = window_handle;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.Windowed = TRUE;

      D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
      UINT               numFeatureLevelsRequested = 1;
      D3D_FEATURE_LEVEL  FeatureLevelsSupported;
      
      // Create a device and a swap chane
      HRESULT hr;
      IDXGISwapChain* g_pSwapChain;
      ID3D11Device* g_pd3dDevice;
      ID3D11DeviceContext* g_pImmediateContext;
      if( FAILED (hr = D3D11CreateDeviceAndSwapChain( NULL, 
                      D3D_DRIVER_TYPE_HARDWARE, 
                      NULL, 
                      0,
                      &FeatureLevelsRequested, 
                      numFeatureLevelsRequested, 
                      D3D11_SDK_VERSION, 
                      &sd, 
                      &g_pSwapChain, 
                      &g_pd3dDevice, 
                      &FeatureLevelsSupported,
                      &g_pImmediateContext )))
      {
        return hr;
      }
      
      // Vertex buffer
      ID3D11Buffer*      g_pVertexBuffer;
      
      // Supply the actual vertex data.
      Vertex g_VertexList[] = {
        { { -0.5f,  0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { {  0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        { {  0.5f,  0.5f, 0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } }
      };

      // Buffer description
      D3D11_BUFFER_DESC bufferDesc;
      bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
      bufferDesc.ByteWidth        = (sizeof( float ) * 3) * 3;
      bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
      bufferDesc.CPUAccessFlags   = 0;
      bufferDesc.MiscFlags        = 0;
      
      // Vertex data
      D3D11_SUBRESOURCE_DATA InitData;
      InitData.pSysMem = g_VertexList;
      InitData.SysMemPitch = 0;
      InitData.SysMemSlicePitch = 0;
      
      // Create vertex buffer
      hr = g_pd3dDevice->CreateBuffer( &bufferDesc, &InitData, &g_pVertexBuffer );

      if (FAILED(hr)) {
        return hr;
      }
      
      // Shape of vertex
      D3D11_INPUT_ELEMENT_DESC g_VertexDesc[] {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      };

    }
  }
  return 0;
}

}
