#include <spvm_native.h>

#include <windows.h>

int32_t SPNATIVE__Cotton__Win__Runtime__run(SPVM_ENV* env, SPVM_VALUE* stack) {
  
  void* sv_self = stack[0].oval;
  
  int32_t e;
  
  void* sv_app = env->get_field_object_by_name(env, sv_self, "Cotton::Win::Runtime", "app", "Cotton::App", &e, __FILE__, __LINE__);
  if (e) { return e; }

  // Create main window
  HWND main_window = COTTON_WIN_RUNTIME_new_main_window(env, sv_app);
  if (main_window == NULL) return -1;
  
  // Get and dispatch message
  MSG message;
  while(GetMessage(&message , NULL , 0 , 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return 0;
}
