#include <spvm_native.h>

enum {
  BOX_SIZING_CONTENT_BOX = 0,
  BOX_SIZING_BORDER_BOX = 1
}

struct spvm__eg__layout__box {
  float color_red;
  float color_green;
  float color_blue;
  float color_alpha;
  int32_t has_background_color;
  float background_color_red;
  float background_color_green;
  float background_color_blue;
  float background_color_alpha;
  int32_t left;
  int32_t top;
  int32_t width;
  int32_t height;
  struct spvm__eg__layout__box* first_child;
  struct spvm__eg__layout__box* last_child;
  struct spvm__eg__layout__box* next_sibling;
  struct spvm__eg__layout__box* parent_node;
  const char* text;
  int8_t box_sizing;
};
