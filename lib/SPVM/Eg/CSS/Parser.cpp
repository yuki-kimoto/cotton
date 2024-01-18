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

static const char* FILE_NAME = "Eg/CSS/Parser.cpp";

extern "C" {

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

int32_t SPVM__Eg__CSS__Parser__build_box_styles(SPVM_ENV* env, SPVM_VALUE* stack) {
  
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

}
