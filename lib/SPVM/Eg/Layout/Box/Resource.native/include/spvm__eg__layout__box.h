#include <spvm_native.h>

enum {
  BOX_SIZING_CONTENT_BOX = 0,
  BOX_SIZING_BORDER_BOX = 1
};

enum {
  HIGHT_AUTO = 1,
};

enum {
  EG_STYLE_VALUE_TYPE_GLOBAL_VALUE,
  EG_STYLE_VALUE_TYPE_GLOBAL_INHERIT,
  EG_STYLE_VALUE_TYPE_GLOBAL_INITIAL,
  EG_STYLE_VALUE_TYPE_GLOBAL_REVERT,
  EG_STYLE_VALUE_TYPE_GLOBAL_UNSET,
  EG_STYLE_VALUE_TYPE_HEIGHT_AUTO,
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
  int8_t box_sizing;
  int8_t is_anon_box;
  int8_t has_background_color;
  int8_t height_value_type;
};
