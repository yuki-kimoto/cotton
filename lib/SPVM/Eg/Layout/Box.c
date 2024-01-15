// Copyright (c) 2023 Yuki Kimoto
// MIT License

#include "spvm_native.h"

#include "spvm__eg__layout__box.h"

static const char* FILE_NAME = "Eg/Layout/Box.cpp";

int32_t SPVM__Eg__Layout__Box__new(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  int32_t error_id = 0;
  
  struct tm* st_layout_box = env->new_memory_block(env, stack, sizeof(struct spvm__eg__layout__box));
  
  void* obj_layout_box = env->new_pointer_object_by_name(env, stack, "Layout::Box", st_layout_box, &error_id, __func__, FILE_NAME, __LINE__);
  if (error_id) { return error_id; }
  
  stack[0].oval = obj_layout_box;
  
  return 0;
}

int32_t SPVM__Eg__Layout__Box__DESTROY(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  void* obj_layout_box = stack[0].oval;
  
  struct spvm__eg__layout__box* st_layout_box = env->get_pointer(env, stack, obj_layout_box);
  env->free_memory_block(env, stack, st_layout_box);
  
  return 0;
}

