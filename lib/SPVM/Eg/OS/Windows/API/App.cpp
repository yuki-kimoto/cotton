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

#include "spvm__eg__layout__box.h"

static const char* FILE_NAME = "Eg/OS/Windows/API/App.cpp";

extern "C" {

static LRESULT CALLBACK window_procedure(HWND window_handle , UINT message , WPARAM wparam , LPARAM lparam);

static int32_t paint_event_handler(SPVM_ENV* env, SPVM_VALUE* stack, void* obj_self);

static void alert(SPVM_ENV* env, SPVM_VALUE* stack, const char* message);

struct Vertex {
        float pos[ 3 ];
        float col[ 4 ];
};

int32_t SPVM__Eg__OS__Windows__API__App__open_main_window_native(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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
  
  void* obj_window_handle = env->new_pointer_object_by_name(env, stack, "Eg::OS::Windows::WindowHandle", window_handle, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  stack[0].oval = obj_self;
  stack[1].oval = env->new_string_nolen(env, stack, "window_handle");
  stack[2].oval = obj_window_handle;
  env->call_instance_method_by_name(env, stack, "set_data", 3, &error_id, __func__, FILE_NAME, __LINE__);
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

int32_t SPVM__Eg__OS__Windows__API__App__CW_USEDEFAULT(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  stack[0].ival = CW_USEDEFAULT;
  
  return 0;
}

int32_t SPVM__Eg__OS__Windows__API__App__start_event_loop(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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
      
      return 0;
    }
    case WM_PAINT: {
      int32_t error_id = 0;
      
      error_id = paint_event_handler(env, stack, obj_self);
      
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
  ID2D1HwndRenderTarget* renderer;
};

static int32_t paint_event_handler(SPVM_ENV* env, SPVM_VALUE* stack, void* obj_self) {
  int32_t error_id = 0;
  
  stack[0].oval = obj_self;
  stack[1].oval = env->new_string_nolen(env, stack, "window_handle");
  env->call_instance_method_by_name(env, stack, "get_data", 2, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_window_handle = stack[0].oval;
  
  HWND window_handle = (HWND)env->get_pointer(env, stack, obj_window_handle);
  
  // Set window text
  {
    stack[0].oval = obj_self;
    env->call_instance_method_by_name(env, stack, "document_title", 1, &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    void* obj_document_title = stack[0].oval;
    
    void* obj_document_title_utf8_to_utf16 = NULL;
    {
      stack[0].oval = obj_document_title;
      env->call_class_method_by_name(env, stack, "Encode", "encode_utf16", 1, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      obj_document_title_utf8_to_utf16 = stack[0].oval;
    }
    int16_t* document_title_utf8_to_utf16 = env->get_elems_short(env, stack, obj_document_title_utf8_to_utf16);
    
    SetWindowTextW(window_handle, (LPCWSTR)document_title_utf8_to_utf16);
  }
  
  // Paint nodes
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
      paint_info->renderer = renderer;
      
      void* obj_paint_info = env->new_pointer_object_by_name(env, stack, "Eg::OS::Windows::PaintInfo", paint_info, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      
      stack[0].oval = obj_self;
      stack[1].oval = obj_paint_info;
      env->call_instance_method_by_name(env, stack, "paint_nodes", 2, &error_id, __func__, FILE_NAME, __LINE__);
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

static int32_t calc_text_height(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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

static int32_t parse_css_length_value (SPVM_ENV* env, SPVM_VALUE* stack, const char* style_value, int32_t style_value_length, int32_t* length) {
  
  int32_t success = 0;
  
  const char* css_length_pattern = "^(\\d+)(px)$";
  
  int32_t css_length_pattern_length = strlen(css_length_pattern);
  
  RE2::Options options;
  options.set_log_errors(false);
  re2::StringPiece stp_css_length_pattern(css_length_pattern, css_length_pattern_length);
  
  std::unique_ptr<RE2> re2(new RE2(stp_css_length_pattern, options));
  
  std::string error = re2->error();
  std::string error_arg = re2->error_arg();
  
  if (!re2->ok()) {
    return success;
  }
  
  int32_t captures_length = re2->NumberOfCapturingGroups();
  int32_t doller0_and_captures_length = captures_length + 1;
  
  int32_t offset = 0;
  
  std::vector<re2::StringPiece> submatch(doller0_and_captures_length);
  int32_t match = re2->Match(style_value, offset, offset + style_value_length, re2::RE2::Anchor::UNANCHORED, submatch.data(), doller0_and_captures_length);
  
  int32_t pixel = 0;
  
  if (match) {
    success = 1;
    
    char* number_string = (char*)env->new_memory_block(env, stack, submatch[1].length() + 1);
    memcpy(number_string, submatch[1].data(), submatch[1].length());
    char* end;
    double number = strtod(number_string, &end);
    
    char* unit = (char*)env->new_memory_block(env, stack, submatch[2].length() + 1);
    memcpy(unit, submatch[2].data(), submatch[2].length());
    
    if (strcmp(unit, "px") == 0) {
      pixel = (int32_t)number;
    }
    
    env->free_memory_block(env, stack, number_string);
    env->free_memory_block(env, stack, unit);
    
    *length = pixel;
  }
  
  return success;
}

static int32_t parse_css_color_value (SPVM_ENV* env, SPVM_VALUE* stack, const char* style_value, int32_t style_value_length, int32_t* style_value_type, float* red, float* green, float* blue, float* alpha) {
  
  int32_t success = 0;
  
  const char* css_color_pattern = "^#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})$";
  
  int32_t css_color_pattern_length = strlen(css_color_pattern);
  
  RE2::Options options;
  options.set_log_errors(false);
  re2::StringPiece stp_css_color_pattern(css_color_pattern, css_color_pattern_length);
  
  std::unique_ptr<RE2> re2(new RE2(stp_css_color_pattern, options));
  
  std::string error = re2->error();
  std::string error_arg = re2->error_arg();
  
  if (!re2->ok()) {
    abort();
  }
  
  int32_t captures_length = re2->NumberOfCapturingGroups();
  int32_t doller0_and_captures_length = captures_length + 1;
  
  int32_t offset = 0;
  
  std::vector<re2::StringPiece> submatch(doller0_and_captures_length);
  int32_t match = re2->Match(style_value, offset, offset + style_value_length, re2::RE2::Anchor::UNANCHORED, submatch.data(), doller0_and_captures_length);
  
  if (match) {
    
    success = 1;
    
    *style_value_type = EG_STYLE_VALUE_TYPE_VALUE;
    
    char* red_string = (char*)env->new_memory_block(env, stack, submatch[1].length() + 1);
    memcpy(red_string, submatch[1].data(), submatch[1].length());
    char* red_end;
    *red = strtol(red_string, &red_end, 16);
    *red /= UINT8_MAX;
    env->free_memory_block(env, stack, red_string);
    
    char* green_string = (char*)env->new_memory_block(env, stack, submatch[2].length() + 1);
    memcpy(green_string, submatch[2].data(), submatch[2].length());
    char* green_end;
    *green = strtol(green_string, &green_end, 16);
    *green /= UINT8_MAX;
    env->free_memory_block(env, stack, green_string);
    
    char* blue_string = (char*)env->new_memory_block(env, stack, submatch[3].length() + 1);
    memcpy(blue_string, submatch[3].data(), submatch[3].length());
    char* blue_end;
    *blue = strtol(blue_string, &blue_end, 16);
    *blue /= UINT8_MAX;
    env->free_memory_block(env, stack, blue_string);
  }
  
  return success;
}

int32_t SPVM__Eg__OS__Windows__API__App__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_paint_info = stack[1].oval;
  void* obj_node = stack[2].oval;
  
  struct COTTON_RUNTIME_PAINT_INFO* paint_info = (struct COTTON_RUNTIME_PAINT_INFO*)env->get_pointer(env, stack, obj_paint_info);
  HDC hdc = paint_info->hdc;
  ID2D1HwndRenderTarget* renderer = paint_info->renderer;
  
  stack[0].oval = obj_node;
  env->call_instance_method_by_name(env, stack, "style_pairs", 1, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_style_pairs = stack[0].oval;
  
  int32_t style_pairs_length = env->length(env, stack, obj_style_pairs);
  
  struct spvm__eg__layout__box css_box = {0};
  
  css_box.left = 0;
  css_box.top = 0;
  css_box.width = 0;
  css_box.height = 0;
  
  // Windows Inner width(viewport)
  {
    stack[0].oval = obj_self;
    env->call_instance_method_by_name(env, stack, "inner_width", 0, &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    css_box.width = stack[0].ival;
  }
  
  stack[0].oval = obj_node;
  env->call_instance_method_by_name(env, stack, "get_text_for_css_box", 1, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_text = stack[0].oval;
  
  if (obj_text) {
    const char* text = env->get_chars(env, stack, obj_text);
    
    stack[0].oval = obj_self;
    stack[1].oval = obj_paint_info;
    stack[2].oval = obj_text;
    stack[3].ival = css_box.width;
    calc_text_height(env, stack);
    if (error_id) { return error_id; }
    css_box.height = stack[0].ival;
    
    spvm_warn("LINE %d %d %s", __LINE__, css_box.height, text);
  }
  
  css_box.has_background_color = 0;
  css_box.background_color_red = 1;
  css_box.background_color_green = 1;
  css_box.background_color_blue = 1;
  css_box.background_color_alpha = 1;
  int32_t has_color = 0;
  css_box.color_red = 0;
  css_box.color_green = 0;
  css_box.color_blue = 0;
  css_box.color_alpha = 1;
  
  for (int32_t i = 0; i < style_pairs_length; i += 2) {
    void* obj_style_name = env->get_elem_object(env, stack, obj_style_pairs, i);
    void* obj_style_value = env->get_elem_object(env, stack, obj_style_pairs, i + 1);
    
    const char* style_name = env->get_chars(env, stack, obj_style_name);
    const char* style_value = env->get_chars(env, stack, obj_style_value);
    int32_t style_value_length = env->length(env, stack, obj_style_value);
    
    switch (style_name[0]) {
      case 'b' : {
        
        if (strcmp(style_name, "background-color") == 0) {
          
          css_box.has_background_color = 1;
          
          int32_t style_value_type = -1;
          int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &css_box.background_color_red, &css_box.background_color_green, &css_box.background_color_blue, &css_box.background_color_alpha);
        }
        
        break;
      }
      case 'c' : {
        
        if (strcmp(style_name, "color") == 0) {
          
          has_color = 1;
          
          int32_t style_value_type = -1;
          int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &css_box.color_red, &css_box.color_green, &css_box.color_blue, &css_box.color_alpha);
        }
        
        break;
      }
      case 'l' : {
        
        if (strcmp(style_name, "left") == 0) {
          parse_css_length_value(env, stack, style_value, style_value_length, &css_box.left);
        }
        
        break;
      }
      case 't' : {
        
        if (strcmp(style_name, "top") == 0) {
          parse_css_length_value(env, stack, style_value, style_value_length, &css_box.top);
        }
        
        break;
      }
      case 'w' : {
        
        if (strcmp(style_name, "width") == 0) {
          parse_css_length_value(env, stack, style_value, style_value_length, &css_box.width);
        }
        
        break;
      }
      case 'h' : {
        
        if (strcmp(style_name, "height") == 0) {
          parse_css_length_value(env, stack, style_value, style_value_length, &css_box.height);
        }
        
        break;
      }
    }
  }
  
  // Block rect
  D2D1_RECT_F block_rect = D2D1::RectF(css_box.left, css_box.top, css_box.left + css_box.width + 1, css_box.top + css_box.height + 1);
  
  // Draw block
  {
    D2D1::ColorF background_color_f = {0};
    
    if (css_box.has_background_color) {
      background_color_f = D2D1::ColorF(css_box.background_color_red, css_box.background_color_green, css_box.background_color_blue, css_box.background_color_alpha);
    }
    else {
      background_color_f = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1);
    }
    
    // Create background brash
    ID2D1SolidColorBrush* background_brush = NULL;
    renderer->CreateSolidColorBrush(
      background_color_f,
      &background_brush
    );
    assert(background_brush);
    
    // 四角形の描画
    renderer->FillRectangle(&block_rect, background_brush);
    
    // ブラシの破棄
    background_brush->Release();
  }
  
  if (obj_text) {
    
    // Render block which has text
    
    const char* text = env->get_chars(env, stack, obj_text);
    
    const int16_t* text_utf16 = encode_utf16(env, stack, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;
    
    // Get parent width and heigth
    // Plus 1 becuase Windows don't contain right and bottom pixcel
    int32_t parent_width = block_rect.right + 1;
    int32_t parent_height = block_rect.bottom + 1;
    
    D2D1::ColorF color_f = {0};
    if (has_color) {
      color_f = D2D1::ColorF(css_box.color_red, css_box.color_green, css_box.color_blue, css_box.color_alpha);
    }
    else {
      color_f = D2D1::ColorF(0, 0, 0, 0);
    }
    
    // draw width
    int32_t width = parent_width;
    
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
        , width    // 枠の幅
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

int32_t SPVM__Eg__OS__Windows__API__App__build_layout_box_styles(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_layout_box = env->get_field_object_by_name(env, stack, obj_node, "layout_box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  struct spvm__eg__layout__box* layout_box = (struct spvm__eg__layout__box*)env->get_pointer(env, stack, obj_layout_box);
  
  stack[0].oval = obj_node;
  env->call_instance_method_by_name(env, stack, "merged_style_pairs", 1, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_style_pairs = stack[0].oval;
  
  int32_t style_pairs_length = env->length(env, stack, obj_style_pairs);
  
  for (int32_t i = 0; i < style_pairs_length; i += 2) {
    void* obj_style_name = env->get_elem_object(env, stack, obj_style_pairs, i);
    void* obj_style_value = env->get_elem_object(env, stack, obj_style_pairs, i + 1);
    
    const char* style_name = env->get_chars(env, stack, obj_style_name);
    const char* style_value = env->get_chars(env, stack, obj_style_value);
    int32_t style_value_length = env->length(env, stack, obj_style_value);
    
    int32_t style_value_type = -1;
    if (strcmp(style_value, "inherit") == 0) {
      style_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    else if (strcmp(style_value, "initial") == 0) {
      style_value_type = EG_STYLE_VALUE_TYPE_INITIAL;
    }
    else if (strcmp(style_value, "revert") == 0) {
      style_value_type = EG_STYLE_VALUE_TYPE_REVERT;
    }
    else if (strcmp(style_value, "unset") == 0) {
      style_value_type = EG_STYLE_VALUE_TYPE_UNSET;
    }
    
    switch (style_name[0]) {
      case 'b' : {
        
        if (strcmp(style_name, "background-color") == 0) {
          
          if (style_value_type > 0) {
            layout_box->background_color_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "currentcolor") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_CURRENTCOLOR;
            }
            else if (strcmp(style_value, "transparent") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_TRANSPARENT;
            }
            else {
              int32_t style_value_type = -1;
              float background_color_red;
              float background_color_green;
              float background_color_blue;
              float background_color_alpha;
              
              int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &background_color_red, &background_color_green, &background_color_blue, &background_color_alpha);
              
              if (success) {
                layout_box->background_color_value_type = style_value_type;
                
                if (style_value_type == EG_STYLE_VALUE_TYPE_VALUE) {
                  layout_box->background_color_red = background_color_red;
                  layout_box->background_color_green = background_color_green;
                  layout_box->background_color_blue = background_color_blue;
                  layout_box->background_color_alpha = background_color_alpha;
                }
              }
              else {
                layout_box->background_color_value_type = EG_STYLE_VALUE_TYPE_TRANSPARENT;
              }
            }
          }
        }
        
        break;
      }
      case 'c' : {
        
        if (strcmp(style_name, "color") == 0) {
          
          if (style_value_type > 0) {
            layout_box->color_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "currentcolor") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_CURRENTCOLOR;
            }
            else {
              int32_t style_value_type = -1;
              float color_red;
              float color_green;
              float color_blue;
              float color_alpha;
              
              int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &color_red, &color_green, &color_blue, &color_alpha);
              
              if (success) {
                if (style_value_type == EG_STYLE_VALUE_TYPE_TRANSPARENT) {
                  layout_box->background_color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
                }
                else {
                  layout_box->background_color_value_type = style_value_type;
                  
                  if (style_value_type == EG_STYLE_VALUE_TYPE_VALUE) {
                    layout_box->color_red = color_red;
                    layout_box->color_green = color_green;
                    layout_box->color_blue = color_blue;
                    layout_box->color_alpha = color_alpha;
                  }
                }
              }
              else {
                layout_box->background_color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
              }
            }
          }
        }
        
        break;
      }
      case 'l' : {
        
        if (strcmp(style_name, "left") == 0) {
          if (style_value_type > 0) {
            layout_box->left_value_type = style_value_type;
          }
          else {
            int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &layout_box->left);
          }
        }
        
        break;
      }
      case 't' : {
        
        if (strcmp(style_name, "top") == 0) {
          if (style_value_type > 0) {
            layout_box->top_value_type = style_value_type;
          }
          else {
            int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &layout_box->top);
          }
        }
        
        break;
      }
      case 'w' : {
        
        if (strcmp(style_name, "width") == 0) {
          if (style_value_type > 0) {
            layout_box->width_value_type = style_value_type;
          }
          else {
            int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &layout_box->width);
          }
        }
        
        break;
      }
      case 'h' : {
        
        if (strcmp(style_name, "height") == 0) {
          if (style_value_type > 0) {
            layout_box->height_value_type = style_value_type;
          }
          else {
            int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &layout_box->height);
          }
        }
        
        break;
      }
    }
  }
  
  if (layout_box->background_color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->background_color_value_type = EG_STYLE_VALUE_TYPE_TRANSPARENT;
  }
  
  if (layout_box->color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
  }
  
  if (layout_box->top_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->top_value_type = EG_STYLE_VALUE_TYPE_AUTO;
  }
  
  if (layout_box->left_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->left_value_type = EG_STYLE_VALUE_TYPE_AUTO;
  }
  
  if (layout_box->width_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->width_value_type = EG_STYLE_VALUE_TYPE_AUTO;
  }
  
  if (layout_box->height_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
    layout_box->height_value_type = EG_STYLE_VALUE_TYPE_AUTO;
  }
  
  return 0;
}

int32_t SPVM__Eg__OS__Windows__API__App__build_layout_box_descendant(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  void* obj_paint_info = stack[2].oval;
  
  void* obj_layout_box = env->get_field_object_by_name(env, stack, obj_node, "layout_box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  struct spvm__eg__layout__box* layout_box = (struct spvm__eg__layout__box*)env->get_pointer(env, stack, obj_layout_box);
  
  void* obj_parent_node = env->get_field_object_by_name(env, stack, obj_node, "parent_node", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_parent_layout_box = NULL;
  struct spvm__eg__layout__box* parent_layout_box = NULL;
  if (obj_parent_node) {
    obj_parent_layout_box = env->get_field_object_by_name(env, stack, obj_node, "layout_box", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    parent_layout_box = (struct spvm__eg__layout__box*)env->get_pointer(env, stack, obj_parent_layout_box);
  }
  
  if (obj_parent_layout_box) {
    if (layout_box->color_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->color_red = parent_layout_box->color_red;
      layout_box->color_green = parent_layout_box->color_green;
      layout_box->color_blue = parent_layout_box->color_blue;
      layout_box->color_alpha = parent_layout_box->color_alpha;
    }
    
    if (layout_box->background_color_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->background_color_red = parent_layout_box->background_color_red;
      layout_box->background_color_green = parent_layout_box->background_color_green;
      layout_box->background_color_blue = parent_layout_box->background_color_blue;
      layout_box->background_color_alpha = parent_layout_box->background_color_alpha;
    }
    
    if (layout_box->left_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->left = parent_layout_box->left;
    }
    
    if (layout_box->top_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->top = parent_layout_box->top;
    }
    
    if (layout_box->width_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->width = parent_layout_box->width;
    }
    
    if (layout_box->height_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      layout_box->height = parent_layout_box->height;
    }
  }
  
  stack[0].oval = obj_node;
  env->call_instance_method_by_name(env, stack, "node_value", 1, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_text = stack[0].oval;
  
  if (obj_text) {
    layout_box->text = env->get_chars(env, stack, obj_text);
    
    stack[0].oval = obj_self;
    stack[1].oval = obj_paint_info;
    stack[2].oval = obj_text;
    stack[3].ival = layout_box->width;
    calc_text_height(env, stack);
    if (error_id) { return error_id; }
    layout_box->height = stack[0].ival;
  }
  
  return 0;
}

int32_t SPVM__Eg__OS__Windows__API__App__build_layout_box_ascendant(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_layout_box = env->get_field_object_by_name(env, stack, obj_node, "layout_box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  struct spvm__eg__layout__box* layout_box = (struct spvm__eg__layout__box*)env->get_pointer(env, stack, obj_layout_box);
  
  void* obj_parent_node = env->get_field_object_by_name(env, stack, obj_node, "parent_node", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_parent_layout_box = NULL;
  struct spvm__eg__layout__box* parent_layout_box = NULL;
  if (obj_parent_node) {
    obj_parent_layout_box = env->get_field_object_by_name(env, stack, obj_node, "layout_box", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    parent_layout_box = (struct spvm__eg__layout__box*)env->get_pointer(env, stack, obj_parent_layout_box);
  }
  
  if (obj_parent_layout_box) {
    if (layout_box->height_value_type == EG_STYLE_VALUE_TYPE_AUTO) {
      parent_layout_box->height = layout_box->height;
    }
  }
  
  return 0;
}

}
