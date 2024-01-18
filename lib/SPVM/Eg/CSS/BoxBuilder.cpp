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

#include "spvm__eg__css__box.h"

static const char* FILE_NAME = "Eg/CSS/BoxBuilder.cpp";

extern "C" {

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

static int32_t parse_css_length_value (SPVM_ENV* env, SPVM_VALUE* stack, const char* style_value, int32_t style_value_length, int32_t* style_value_type, double* length) {
  
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
  
  int32_t length_tmp = 0;
  
  if (match) {
    success = 1;
    
    *style_value_type = EG_STYLE_VALUE_TYPE_VALUE;
    
    char* length_string = (char*)env->new_memory_block(env, stack, submatch[1].length() + 1);
    memcpy(length_string, submatch[1].data(), submatch[1].length());
    char* end;
    double length_tmp = strtod(length_string, &end);
    
    char* unit = (char*)env->new_memory_block(env, stack, submatch[2].length() + 1);
    memcpy(unit, submatch[2].data(), submatch[2].length());
    
    if (strcmp(unit, "px") == 0) {
      // Do nothing
    }
    
    env->free_memory_block(env, stack, length_string);
    env->free_memory_block(env, stack, unit);
    
    *length =length_tmp;
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
    
    *alpha = 1;
  }
  
  return success;
}

int32_t SPVM__Eg__CSS__BoxBuilder__paint_node(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  
  void* obj_node = stack[1].oval;
  
  if (!obj_node) {
    return env->die(env, stack, "$node must be defined.", __func__, FILE_NAME, __LINE__);
  }
  
  void* obj_box = env->get_field_object_by_name(env, stack, obj_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  if (!obj_box) {
    return 0;
  }
  
  struct spvm__eg__css__box* box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_box);
  
  D2D1_RECT_F box_rect = D2D1::RectF(box->left, box->top, box->left + box->width + 1, box->top + box->height + 1);
  
  // Renderer
  stack[0].oval = obj_self;
  stack[1].oval = env->new_string_nolen(env, stack, "renderer");
  env->call_instance_method_by_name(env, stack, "get_data", 2, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  void* obj_renderer = stack[0].oval;
  assert(obj_renderer);
  ID2D1HwndRenderTarget* renderer = (ID2D1HwndRenderTarget*)env->get_pointer(env, stack, obj_renderer);
  
  // Draw box
  if (!(box->background_color_alpha == 0)) {
    
    D2D1::ColorF background_color_f = {0};
    
    background_color_f = D2D1::ColorF(box->background_color_red, box->background_color_green, box->background_color_blue, box->background_color_alpha);
    
    
    ID2D1SolidColorBrush* background_brush = NULL;
    renderer->CreateSolidColorBrush(
      background_color_f,
      &background_brush
    );
    assert(background_brush);
    
    renderer->FillRectangle(&box_rect, background_brush);
    
    background_brush->Release();
  }
  
  // Draw text
  const char* text = box->text;
  if (text) {
    
    int32_t font_size = box->font_size;
    DWRITE_FONT_WEIGHT font_weight_native = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE font_style_native = DWRITE_FONT_STYLE_NORMAL;
    
    const int16_t* text_utf16 = encode_utf16(env, stack, text);
    int32_t text_utf16_length = strlen((char*)text_utf16) / 2;
    
    D2D1::ColorF color_f = {0};
    color_f = D2D1::ColorF(box->color_red, box->color_green, box->color_blue, box->color_alpha);
    
    HRESULT hresult = E_FAIL;
    
    IDWriteFactory* direct_write_factory = NULL;
    hresult = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>( &direct_write_factory ) );
    
    if (FAILED(hresult)) {
      return env->die(env, stack, "DWriteCreateFactory() failed.", __func__, FILE_NAME, __LINE__);
    }
    
    IDWriteTextFormat* text_format = NULL;
    direct_write_factory->CreateTextFormat(
      L"Meiryo",
      NULL,
      font_weight_native,
      font_style_native,
      DWRITE_FONT_STRETCH_NORMAL,
      font_size,
      L"",
      &text_format
    );
    
    IDWriteTextLayout* text_layout = NULL;
    hresult = direct_write_factory->CreateTextLayout(
          (const WCHAR*)text_utf16
        , text_utf16_length
        ,text_format
        , box->width
        , 0
        , &text_layout
    );
    
    if (FAILED(hresult)) {
      return env->die(env, stack, "IDWriteFactory#CreateTextLayout() failed.", __func__, FILE_NAME, __LINE__);
    }
    
    ID2D1SolidColorBrush* text_brush = NULL;
    renderer->CreateSolidColorBrush(
      color_f,
      &text_brush
    );
    
    D2D1_POINT_2F point = {.x = (float)box_rect.left, .y = (float)box_rect.top};
    renderer->DrawTextLayout(point, text_layout, text_brush);
  }
  
  return 0;
}

int32_t SPVM__Eg__CSS__BoxBuilder__build_box_styles(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_box = env->get_field_object_by_name(env, stack, obj_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  assert(obj_box);
  
  struct spvm__eg__css__box* box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_box);
  
  assert(box);
  
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
    
    int32_t style_value_type = EG_STYLE_VALUE_TYPE_UNKNOWN;
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
          
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->background_color_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "currentcolor") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_CURRENTCOLOR;
            }
            else if (strcmp(style_value, "transparent") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_TRANSPARENT;
            }
            else {
              int32_t style_value_type = EG_STYLE_VALUE_TYPE_UNKNOWN;
              float background_color_red;
              float background_color_green;
              float background_color_blue;
              float background_color_alpha;
              int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &background_color_red, &background_color_green, &background_color_blue, &background_color_alpha);
              
              if (success) {
                box->background_color_value_type = style_value_type;
                
                box->background_color_red = background_color_red;
                box->background_color_green = background_color_green;
                box->background_color_blue = background_color_blue;
                box->background_color_alpha = background_color_alpha;
              }
            }
          }
        }
        
        break;
      }
      case 'c' : {
        
        if (strcmp(style_name, "color") == 0) {
          
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->color_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "currentcolor") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_CURRENTCOLOR;
            }
            else {
              int32_t style_value_type = EG_STYLE_VALUE_TYPE_UNKNOWN;
              float color_red;
              float color_green;
              float color_blue;
              float color_alpha;
              
              int32_t success = parse_css_color_value(env, stack, style_value, style_value_length, &style_value_type, &color_red, &color_green, &color_blue, &color_alpha);
              
              if (success) {
                box->color_value_type = style_value_type;
                
                if (style_value_type == EG_STYLE_VALUE_TYPE_VALUE) {
                  box->color_red = color_red;
                  box->color_green = color_green;
                  box->color_blue = color_blue;
                  box->color_alpha = color_alpha;
                }
              }
            }
          }
        }
        
        break;
      }
      case 'f' : {
        
        if (strcmp(style_name, "font-size") == 0) {
                spvm_warn("LINE %d", __LINE__);
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->font_size_value_type = style_value_type;
          }
          else {
            int32_t style_value_type = EG_STYLE_VALUE_TYPE_UNKNOWN;
            double font_size;
            
            int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &style_value_type, &font_size);
            
            if (success) {
              box->font_size_value_type = style_value_type;
              
              if (style_value_type == EG_STYLE_VALUE_TYPE_VALUE) {
                box->font_size = (float)font_size;
                spvm_warn("LINE %d %f", __LINE__, box->font_size);
              }
            }
          }
        }
        
        break;
      }
      case 'l' : {
        
        if (strcmp(style_name, "left") == 0) {
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->left_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "auto") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_AUTO;
            }
            else {
              double left;
              int32_t style_value_type = 0;
              int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &style_value_type, &left);
              
              if (success) {
                box->left_value_type = style_value_type;
                box->left = (int32_t)left;
              }
            }
          }
        }
        
        break;
      }
      case 't' : {
        
        if (strcmp(style_name, "top") == 0) {
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->top_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "auto") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_AUTO;
            }
            else {
              double top;
              int32_t style_value_type = 0;
              int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &style_value_type, &top);
              
              if (success) {
                box->top_value_type = style_value_type;
                box->top = (int32_t)top;
              }
            }
          }
        }
        
        break;
      }
      case 'w' : {
        
        if (strcmp(style_name, "width") == 0) {
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->width_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "auto") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_AUTO;
            }
            else {
              double width;
              int32_t style_value_type = 0;
              int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &style_value_type, &width);
              
              if (success) {
                box->width_value_type = style_value_type;
                box->width = (int32_t)width;
              }
            }
          }
        }
        
        break;
      }
      case 'h' : {
        
        if (strcmp(style_name, "height") == 0) {
          if (!(style_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN)) {
            box->height_value_type = style_value_type;
          }
          else {
            if (strcmp(style_value, "auto") == 0) {
              style_value_type = EG_STYLE_VALUE_TYPE_AUTO;
            }
            else {
              double height;
              int32_t style_value_type = 0;
              int32_t success = parse_css_length_value(env, stack, style_value, style_value_length, &style_value_type, &height);
              
              if (success) {
                box->height_value_type = style_value_type;
                box->height = (int32_t)height;
              }
            }
          }
        }
        
        break;
      }
    }
  }
  
  return 0;
}

int32_t SPVM__Eg__CSS__BoxBuilder__build_box_set_default_values(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_box = env->get_field_object_by_name(env, stack, obj_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  assert(obj_box);
  
  struct spvm__eg__css__box* box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_box);
  
  assert(box);
  
  int32_t is_anon_box = 0;
  if (env->is_type_by_name(env, stack, obj_node, "Eg::Text", 0)) {
    is_anon_box = 1;
  }
  
  // Initial value
  if (is_anon_box) {
    if (box->background_color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->background_color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->top_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->top_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->left_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->left_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->width_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->width_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->height_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->height_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->font_size_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->font_size_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
  }
  else {
    if (box->background_color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->background_color_value_type = EG_STYLE_VALUE_TYPE_TRANSPARENT;
    }
    
    if (box->color_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->color_value_type = EG_STYLE_VALUE_TYPE_INHERIT;
    }
    
    if (box->top_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->top_value_type = EG_STYLE_VALUE_TYPE_AUTO;
    }
    
    if (box->left_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->left_value_type = EG_STYLE_VALUE_TYPE_AUTO;
    }
    
    if (box->width_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->width_value_type = EG_STYLE_VALUE_TYPE_AUTO;
    }
    
    if (box->height_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->height_value_type = EG_STYLE_VALUE_TYPE_AUTO;
    }
    
    if (box->font_size_value_type == EG_STYLE_VALUE_TYPE_UNKNOWN) {
      box->font_size_value_type = EG_STYLE_VALUE_TYPE_VALUE;
      box->font_size = 16;
    }
  }
  
  return 0;
}

int32_t SPVM__Eg__CSS__BoxBuilder__build_box_descendant(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_box = env->get_field_object_by_name(env, stack, obj_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  struct spvm__eg__css__box* box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_box);
  
  void* obj_parent_node = env->get_field_object_by_name(env, stack, obj_node, "parent_node", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_parent_box = NULL;
  
  // Not document node
  if (obj_parent_node) {
    int32_t is_root_node = env->is_type_by_name(env, stack, obj_parent_node, "Eg::Document", 0);
    
    obj_parent_box = env->get_field_object_by_name(env, stack, obj_parent_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    struct spvm__eg__css__box* parent_box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_parent_box);
    
    if (box->color_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      if (is_root_node) {
        box->color_red = 0;
        box->color_green = 0;
        box->color_blue = 0;
        box->color_alpha = 1;
      }
      else {
        box->color_red = parent_box->color_red;
        box->color_green = parent_box->color_green;
        box->color_blue = parent_box->color_blue;
        box->color_alpha = parent_box->color_alpha;
        spvm_warn("LINE %d %d %d %d", __LINE__, box->color_red, box->color_green, box->color_blue, box->color_alpha);
      }
    }
    spvm_warn("LINE %d %d %d %d", __LINE__, box->color_red, box->color_green, box->color_blue, box->color_alpha);
    
    if (box->background_color_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      if (is_root_node) {
        box->color_red = 1;
        box->color_green = 1;
        box->color_blue = 1;
        box->color_alpha = 1;
      }
      else {
        box->background_color_red = parent_box->background_color_red;
        box->background_color_green = parent_box->background_color_green;
        box->background_color_blue = parent_box->background_color_blue;
        box->background_color_alpha = parent_box->background_color_alpha;
      }
    }
    
    if (box->left_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      box->left = parent_box->left;
    }
    
    if (box->top_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      box->top = parent_box->top;
    }
    
    if (box->width_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      box->width = parent_box->width;
    }
    else if (box->width_value_type == EG_STYLE_VALUE_TYPE_AUTO) {
      if (is_root_node) {
        stack[0].oval = obj_self;
        env->call_instance_method_by_name(env, stack, "inner_width", 0, &error_id, __func__, FILE_NAME, __LINE__);
        if (error_id) { return error_id; }
        int32_t inner_width = stack[0].ival;
        
        box->width = inner_width;
      }
      else {
        box->width = parent_box->width;
      }
    }
    
    if (box->height_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      box->height = parent_box->height;
    }
    
    if (box->font_size_value_type == EG_STYLE_VALUE_TYPE_INHERIT) {
      box->font_size = parent_box->font_size;
    }
  }
  
  return 0;
}

int32_t SPVM__Eg__CSS__BoxBuilder__build_box_ascendant(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  void* obj_self = stack[0].oval;
  void* obj_node = stack[1].oval;
  
  void* obj_box = env->get_field_object_by_name(env, stack, obj_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  void* obj_parent_node = env->get_field_object_by_name(env, stack, obj_node, "parent_node", &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
    
  // Not document node
  if (obj_parent_node) {
    struct spvm__eg__css__box* box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_box);
    
    void* obj_parent_box = env->get_field_object_by_name(env, stack, obj_parent_node, "box", &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    struct spvm__eg__css__box* parent_box = (struct spvm__eg__css__box*)env->get_pointer(env, stack, obj_parent_box);
    
    stack[0].oval = obj_node;
    env->call_instance_method_by_name(env, stack, "node_value", 1, &error_id, __func__, FILE_NAME, __LINE__);
    if (error_id) { return error_id; }
    void* obj_text = stack[0].oval;
    
    if (obj_text) {
      box->text = env->get_chars(env, stack, obj_text);
    }
    
    if (box->text) {
      stack[0].oval = obj_self;
      stack[1].oval = obj_node;
      env->call_instance_method_by_name(env, stack, "text_metrics_height", 2, &error_id, __func__, FILE_NAME, __LINE__);
      if (error_id) { return error_id; }
      box->height = stack[0].ival;
      spvm_warn("LINE %d %d %d", __LINE__, box->width, box->height);
    }
    
    if (parent_box->height_value_type == EG_STYLE_VALUE_TYPE_AUTO) {
      parent_box->height = box->height;
    }
  }
  
  return 0;
}

}
