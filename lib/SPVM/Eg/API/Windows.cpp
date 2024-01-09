#include <spvm_native.h>

#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <assert.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include<memory>

#include <iostream>

#include "re2/re2.h"

static const char* FILE_NAME = "Eg/API/Windows.cpp";

extern "C" {

static LRESULT CALLBACK window_procedure(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam);

static int32_t repaint(SPVM_ENV* env, SPVM_VALUE* stack, void* obj_self);

static void alert(SPVM_ENV* env, SPVM_VALUE* stack, const char* message);

struct Vertex {
        float pos[ 3 ];
        float col[ 4 ];
};

int32_t SPVM__Eg__API__Windows__open_main_window_native(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  void* obj_self = stack[0].oval;
  
  int32_t window_left = stack[1].ival;
  
  int32_t window_top = stack[2].ival;
  
  int32_t window_width = stack[3].ival;
  
  int32_t window_height = stack[4].ival;
  
  int32_t error_id = 0;
  
  HINSTANCE instance_handle = GetModuleHandle(NULL);
  
  // Register Window Class
  WNDCLASS winc;
  winc.style = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc = window_procedure;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance = instance_handle;
  winc.hIcon = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName = TEXT("Window");
  if (!RegisterClass(&winc)) { return env->die(env, stack, "Can't register the window class"); };
  
  // Create Main Window
  const int16_t* window_class_name = (const int16_t*)TEXT("Window");
  const int16_t* window_title = NULL;
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  HWND window_parent_window_handle = NULL;
  HMENU window_id = NULL;
  void** wm_create_args = (void**)calloc(3, sizeof(void*));
  wm_create_args[0] = env;
  wm_create_args[1] = obj_self;
  wm_create_args[2] = stack;
  void* window_wm_create_lparam = (void*)wm_create_args;
  HWND window_handle = CreateWindow(
    (LPCWSTR)window_class_name, (LPCWSTR)window_title,
    window_style,
    window_left, window_top,
    window_width, window_height,
    window_parent_window_handle, window_id, instance_handle, window_wm_create_lparam
  );
  
  void* obj_window_handle = env->new_pointer_object_by_name(env, stack, "Eg::API::Windows", window_handle, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  env->set_field_object_by_name(env, stack, obj_self, "window_handle", obj_window_handle, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  // Renderer
  {
    RECT window_rect;
    GetWindowRect(window_handle, &window_rect);
    
    int window_width = window_rect.right - window_rect.left;
    int window_height = window_rect.bottom - window_rect.top;
    
    // Swap chain descriptor
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = window_width;
    sd.BufferDesc.Height = window_height;
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
  
  return 0;
}

int32_t SPVM__Eg__API__Windows__CW_USEDEFAULT(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  stack[0].ival = CW_USEDEFAULT;
  
  return 0;
}

int32_t SPVM__Eg__API__Windows__start_app(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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

static LRESULT CALLBACK window_procedure(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam) {
  
  static SPVM_ENV* env;
  static SPVM_VALUE* stack;
  static void* obj_self;
  
  switch (message) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE: {
      CREATESTRUCT* create_struct = (CREATESTRUCT*)lparam;
      void** wm_create_args = (void**)create_struct->lpCreateParams;
      env = (SPVM_ENV*)wm_create_args[0];
      obj_self = (void*)wm_create_args[1];
      stack = (SPVM_VALUE*)wm_create_args[2];
      
      // alert(env, stack, "ハローワールド");
      return 0;
    }
    case WM_PAINT: {
      int32_t error_id = 0;
      
      // Draw node
      error_id = repaint(env, stack, obj_self);
      
      if (error_id) {
        alert(env, stack, env->get_chars(env, stack, env->get_exception(env, stack)));
        PostQuitMessage(0);
        return 0;
      }
      
      return 0;
    }
  }
  return DefWindowProc(window_handle , message , wparam , lparam);
}

static int16_t* encode_utf16(SPVM_ENV* env, SPVM_VALUE* stack, const char* string) {
  int32_t error_id = 0;
  
  void* obj_string =  env->new_string_nolen(env, stack, string);
  
  void* obj_string_utf8_to_utf16 = NULL;
  {
    stack[0].oval = obj_string;
    env->call_class_method_by_name(env, stack, "Encode", "encode_utf16", 1, &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return NULL; }
    obj_string_utf8_to_utf16 = stack[0].oval;
  }
  
  int16_t* string_utf8_to_utf16 = env->get_elems_short(env, stack, obj_string_utf8_to_utf16);
  
  return string_utf8_to_utf16;
}

static void alert(SPVM_ENV* env, SPVM_VALUE* stack, const char* message) {
  int16_t* message_utf8_to_utf16 = encode_utf16(env, stack, message);
  
  MessageBoxW(NULL, (LPCWSTR)message_utf8_to_utf16, TEXT("Alert"), MB_OK);
}

struct COTTON_RUNTIME_PAINT_INFO {
  HDC hdc;
  HWND window_handle;
  ID2D1HwndRenderTarget* renderer;
};

struct EG_STYLE_INFO {
  float color_red;
  float color_green;
  float color_blue;
  float color_alpha;
  float background_color_red;
  float background_color_green;
  float background_color_blue;
  float background_color_alpha;
  int32_t left;
  int32_t top;
  int32_t width;
  int32_t height;
  struct EG_STYLE_INFO* parent;
};

static int32_t repaint(SPVM_ENV* env, SPVM_VALUE* stack, void* obj_self) {
  int32_t error_id = 0;
  
  void* obj_window_handle = env->get_field_object_by_name(env, stack, obj_self, "window_handle", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_app = env->get_field_object_by_name(env, stack, obj_self, "app", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_runtime = env->get_field_object_by_name(env, stack, obj_self, "runtime", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, obj_window_handle);
  
  // Draw page title
  {
    void* obj_app_name = env->new_string(env, stack, "コットン", strlen("コットン"));
    
    void* obj_app_name_utf8_to_utf16 = NULL;
    {
      stack[0].oval = obj_app_name;
      env->call_class_method_by_name(env, stack, "Encode", "encode_utf16", 1, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      obj_app_name_utf8_to_utf16 = stack[0].oval;
    }
    int16_t* app_name_utf8_to_utf16 = env->get_elems_short(env, stack, obj_app_name_utf8_to_utf16);
    
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
    
    {
      int32_t scope = env->enter_scope(env, stack);
      
      struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)calloc(1, sizeof(struct COTTON_RUNTIME_PAINT_INFO));
      paint_info->hdc = hdc;
      paint_info->window_handle = window_handle;
      paint_info->renderer = renderer;
      
      void* obj_paint_info = env->new_pointer_object_by_name(env, stack, "Eg::API::Windows::PaintInfo", paint_info, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      
      stack[0].oval = obj_self;
      stack[1].oval = obj_app;
      stack[2].oval = obj_paint_info;
      env->call_instance_method_by_name(env, stack, "repaint", 3, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      
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

int32_t SPVM__Eg__API__Windows__calc_text_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_paint_info = stack[1].oval;
  void* obj_text = stack[2].oval;
  int32_t draw_width = stack[3].ival;
  void* obj_font_styles = stack[4].oval;

  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, stack, obj_paint_info);
  HDC hdc = paint_info->hdc;
  
  int32_t draw_height = 0;
  if (obj_text) {
    // Render block which has text
    const char* text = env->get_chars(env, stack, obj_text);
    
    const int16_t* text_utf16 = encode_utf16(env, stack, text);
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

static double parse_css_length (SPVM_ENV* env, SPVM_VALUE* stack, const char* style_value, int32_t style_value_length) {
  
  const char* css_length_pattern = "^(\\d+)(px)$";
  
  int32_t css_length_pattern_length = strlen(css_length_pattern);
  
  RE2::Options options;
  options.set_log_errors(false);
  re2::StringPiece stp_css_length_pattern(css_length_pattern, css_length_pattern_length);
  
  std::unique_ptr<RE2> re2(new RE2(stp_css_length_pattern, options));
  
  std::string error = re2->error();
  std::string error_arg = re2->error_arg();
  
  if (!re2->ok()) {
    return env->die(env, stack, "The regex pattern %s can't be compiled. [Error]%s. [Fragment]%s", css_length_pattern, error.data(), error_arg.data(), __func__, FILE_NAME, __LINE__);
  }
  
  int32_t captures_length = re2->NumberOfCapturingGroups();
  int32_t doller0_and_captures_length = captures_length + 1;
  
  int32_t offset = 0;
  
  std::vector<re2::StringPiece> submatch(doller0_and_captures_length);
  int32_t match = re2->Match(style_value, offset, offset + style_value_length, re2::RE2::Anchor::UNANCHORED, submatch.data(), doller0_and_captures_length);
  
  int32_t pixel = 0;
  
  if (match) {
    char* number_string = (char*)env->new_memory_block(env, stack, submatch[0].length() + 1);
    memcpy(number_string, submatch[1].data(), submatch[1].length());
    char* end;
    double number = strtod(number_string, &end);
    
    char* unit = (char*)env->new_memory_block(env, stack, submatch[1].length() + 1);
    memcpy(unit, submatch[2].data(), submatch[2].length());
    
    if (strcmp(unit, "px") == 0) {
      pixel = (int32_t)number;
    }
    
    env->free_memory_block(env, stack, number_string);
    env->free_memory_block(env, stack, unit);
    
  }
  
  return pixel;
}

int32_t SPVM__Eg__API__Windows__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_paint_info = stack[1].oval;
  void* obj_node = stack[2].oval;
  
  void* obj_app = env->get_field_object_by_name(env, stack, obj_self, "app", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, stack, obj_paint_info);
  HDC hdc = paint_info->hdc;
  ID2D1HwndRenderTarget* renderer = paint_info->renderer;
  
  stack[0].oval = obj_node;
  env->call_instance_method_by_name(env, stack, "style_to_pairs", 1, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_style_pairs = stack[0].oval;
  
  int32_t style_pairs_length = env->length(env, stack, obj_style_pairs);
  
  int32_t left = 0;
  int32_t top = 0;
  int32_t width = 0;
  int32_t height = 0;
  for (int32_t i = 0; i < style_pairs_length; i += 2) {
    void* obj_style_name = env->get_elem_object(env, stack, obj_style_pairs, i);
    void* obj_style_value = env->get_elem_object(env, stack, obj_style_pairs, i + 1);
    
    const char* style_name = env->get_chars(env, stack, obj_style_name);
    const char* style_value = env->get_chars(env, stack, obj_style_value);
    int32_t style_value_length = env->length(env, stack, obj_style_value);
    
    switch (style_name[0]) {
      case 'l' : {
        
        if (strcmp(style_name, "left") == 0) {
          left = (int32_t)parse_css_length(env, stack, style_value, style_value_length);
        }
        
        break;
      }
      case 't' : {
        
        if (strcmp(style_name, "top") == 0) {
          top = (int32_t)parse_css_length(env, stack, style_value, style_value_length);
        }
        
        break;
      }
      case 'w' : {
        
        if (strcmp(style_name, "width") == 0) {
          width = (int32_t)parse_css_length(env, stack, style_value, style_value_length);
        }
        
        break;
      }
      case 'h' : {
        
        if (strcmp(style_name, "height") == 0) {
          height = (int32_t)parse_css_length(env, stack, style_value, style_value_length);
        }
        
        break;
      }
    }
  }
  
  int32_t draw_left = env->get_field_int_by_name(env, stack, obj_node, "draw_left", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  int32_t draw_top = env->get_field_int_by_name(env, stack, obj_node, "draw_top", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  int32_t draw_width = env->get_field_int_by_name(env, stack, obj_node, "draw_width", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  int32_t draw_height = env->get_field_int_by_name(env, stack, obj_node, "draw_height", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }

  // Block rect
  D2D1_RECT_F block_rect = D2D1::RectF(draw_left, draw_top, draw_width, draw_height);
  
  printf("EEEEE %f %f\n", block_rect.left, block_rect.right);

  // Draw block
  {
    void* obj_background_color = env->get_field_object_by_name(env, stack, obj_node, "background_color", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    
    int32_t background_color;
    D2D1::ColorF background_color_f = {0};
    if (obj_background_color) {
      float background_color_red = env->get_field_float_by_name(env, stack,  obj_background_color, "red", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float background_color_green = env->get_field_float_by_name(env, stack,  obj_background_color, "green", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float background_color_blue = env->get_field_float_by_name(env, stack,  obj_background_color, "blue", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float background_color_alpha = env->get_field_float_by_name(env, stack,  obj_background_color, "alpha", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

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
    printf("GGGG %d %d %d %d\n", left, top, width, height);
    
    // 四角形の描画
    renderer->FillRectangle(&block_rect, background_brush);
    
    // ブラシの破棄
    background_brush->Release();
  }
  
  void* obj_text_buffer = env->get_field_object_by_name(env, stack, obj_node, "text_buffer", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  if (obj_text_buffer) {
    
    // Render block which has text
    
    stack[0].oval = obj_text_buffer;
    env->call_instance_method_by_name(env, stack, "to_string", 0, &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    void* obj_text = stack[0].oval;
    
    const char* text = env->get_chars(env, stack, obj_text);
    
    const int16_t* text_utf16 = encode_utf16(env, stack, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;

    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = block_rect.right + 1;
    int32_t parent_height = block_rect.bottom + 1;

    void* obj_color = env->get_field_object_by_name(env, stack, obj_node, "color", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    
    int32_t color;
    D2D1::ColorF color_f = D2D1::ColorF(0, 0, 1.0f, 1.0f);
    if (obj_color) {
      float color_red = env->get_field_float_by_name(env, stack,  obj_color, "red", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float color_green = env->get_field_float_by_name(env, stack,  obj_color, "green", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float color_blue = env->get_field_float_by_name(env, stack,  obj_color, "blue", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

      float color_alpha = env->get_field_float_by_name(env, stack,  obj_color, "alpha", &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }

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

int32_t SPVM__Eg__API__Windows__get_viewport_width(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  
  void* obj_window_handle = env->get_field_object_by_name(env, stack, obj_self, "window_handle", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, obj_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);
  
  stack[0].ival = rect.right + 1;
  
  return 0;
}


int32_t SPVM__Eg__API__Windows__get_viewport_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  
  void* obj_window_handle = env->get_field_object_by_name(env, stack, obj_self, "window_handle", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, obj_window_handle);
  
  RECT rect;
  GetClientRect(window_handle, &rect);
  
  stack[0].ival = rect.bottom + 1;
  
  return 0;
}

}
