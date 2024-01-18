#include <spvm_native.h>

enum {
  BOX_SIZING_CONTENT_BOX = 0,
  BOX_SIZING_BORDER_BOX = 1
};

enum {
  HIGHT_AUTO = 1,
};

enum {
  EG_STYLE_VALUE_TYPE_UNKNOWN,
  EG_STYLE_VALUE_TYPE_VALUE,
  EG_STYLE_VALUE_TYPE_INHERIT,
  EG_STYLE_VALUE_TYPE_INITIAL,
  EG_STYLE_VALUE_TYPE_REVERT,
  EG_STYLE_VALUE_TYPE_UNSET,
  EG_STYLE_VALUE_TYPE_AUTO,
  EG_STYLE_VALUE_TYPE_TRANSPARENT,
  EG_STYLE_VALUE_TYPE_CURRENTCOLOR,
};

struct spvm__eg__layout__box {
  struct spvm__eg__layout__box* first_child;
  struct spvm__eg__layout__box* last_child;
  struct spvm__eg__layout__box* next_sibling;
  struct spvm__eg__layout__box* parent_node;
  const char* text;
  int32_t left;
  int32_t top;
  int32_t width;
  int32_t height;
  float color_red;
  float color_green;
  float color_blue;
  float color_alpha;
  float background_color_red;
  float background_color_green;
  float background_color_blue;
  float background_color_alpha;
  float font_size;
  int8_t box_sizing;
  int8_t is_anon_box;
  int8_t left_value_type;
  int8_t top_value_type;
  int8_t width_value_type;
  int8_t height_value_type;
  int8_t color_value_type;
  int8_t background_color_value_type;
  int8_t font_size_value_type;
};
