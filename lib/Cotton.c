#include <spvm_native.h>

#include <windows.h>
#include <png.h>

typedef struct{
  unsigned char *data;
  unsigned int width;
  unsigned int height;
  unsigned int ch;
} BITMAPDATA_t;

enum {
  COTTON_C_SIZE_UNIT_PIXEL,
  COTTON_C_SIZE_UNIT_PERCENT,
};

int pngFileReadDecode(BITMAPDATA_t *bitmapData, const char* filename);
int pngFileReadDecode(BITMAPDATA_t *, const char*);
int pngFileEncodeWrite(BITMAPDATA_t *, const char*);
int freeBitmapData(BITMAPDATA_t *);

#define SIGNATURE_NUM 8

LRESULT CALLBACK WndProc(HWND hwnd , UINT msg , WPARAM wp , LPARAM lp) {

  switch (msg) {
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd , &ps);

      RECT client_rect;
      GetClientRect(hwnd , &client_rect);
      {
        HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0x00, 0xAA, 0x77));
        SelectObject(hdc, hpen);
        HBRUSH brash = CreateSolidBrush(RGB(0x00, 0xAA, 0x77));
        SelectObject(hdc, brash);
        Rectangle(hdc, 0, 0, client_rect.right, 100);
        DeleteObject(hpen);
        DeleteObject(brash);
      }
      {
        HPEN hpen = CreatePen(PS_SOLID , 0 , RGB(0xEE, 0x00, 0x7F));
        SelectObject(hdc, hpen);
        HBRUSH brash = CreateSolidBrush(RGB(0xEE, 0x00, 0x7F));
        SelectObject(hdc, brash);
        Rectangle(hdc, 0, 100, client_rect.right, 200);
        DeleteObject(hpen);
        DeleteObject(brash);
      }
      
      {
        LOGFONT lfFont;
        lfFont.lfHeight     = 40;
        lfFont.lfWidth = lfFont.lfEscapement =
        lfFont.lfOrientation    = 0;
        lfFont.lfWeight     = FW_BOLD;
        lfFont.lfItalic = lfFont.lfUnderline = FALSE;
        lfFont.lfStrikeOut    = FALSE; 
        lfFont.lfCharSet    = SHIFTJIS_CHARSET;
        lfFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
        lfFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        lfFont.lfQuality    = DEFAULT_QUALITY;
        lfFont.lfPitchAndFamily = 0;
        lfFont.lfFaceName[0]    = '\0';
        HFONT hFont = CreateFontIndirect(&lfFont);
        
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
        SetBkMode(hdc , TRANSPARENT);
        TextOut(hdc , 0 , 0 , "Cotton" , lstrlen("Cotton"));
        DeleteObject(hFont);
      }

      EndPaint(hwnd , &ps);
      
      return 0;
    }
  }
  return DefWindowProc(hwnd , msg , wp , lp);
}

int WINAPI WinMain(HINSTANCE hInstance , HINSTANCE hPrevInstance ,
      PSTR lpCmdLine , int nCmdShow ) {

  BITMAPDATA_t bitmap;
  const char* png_file = "kaeru_w_01.png";
  pngFileReadDecode(&bitmap, png_file);
  
  WNDCLASS winc;
  winc.style    = CS_HREDRAW | CS_VREDRAW;
  winc.lpfnWndProc  = WndProc;
  winc.cbClsExtra = winc.cbWndExtra = 0;
  winc.hInstance    = hInstance;
  winc.hIcon    = LoadIcon(NULL , IDI_APPLICATION);
  winc.hCursor    = LoadCursor(NULL , IDC_ARROW);
  winc.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
  winc.lpszMenuName = NULL;
  winc.lpszClassName  = TEXT("Cotton");

  if (!RegisterClass(&winc)) return -1;

  HWND hwnd = CreateWindow(
      TEXT("Cotton") , TEXT("Cotton") ,
      WS_OVERLAPPEDWINDOW | WS_VISIBLE ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      CW_USEDEFAULT , CW_USEDEFAULT ,
      NULL , NULL , hInstance , NULL
  );

  if (hwnd == NULL) return -1;

  MSG msg;
  while(GetMessage(&msg , NULL , 0 , 0)) DispatchMessage(&msg);
  return msg.wParam;
}

int pngFileReadDecode(BITMAPDATA_t *bitmapData, const char* filename){

  FILE *fi;
  int j;
  unsigned int width, height;
  unsigned int readSize;

  png_structp png;
  png_infop info;
  png_bytepp datap;
  png_byte type;
  png_byte signature[8];

  fi = fopen(filename, "rb");
  if(fi == NULL){
    printf("%sは開けません\n", filename);
    return -1;
  }

  readSize = fread(signature, 1, SIGNATURE_NUM, fi);

  if(png_sig_cmp(signature, 0, SIGNATURE_NUM)){
    printf("png_sig_cmp error!\n");
    fclose(fi);
    return -1;
  }

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png == NULL){
    printf("png_create_read_struct error!\n");
    fclose(fi);
    return -1;
  }

  info = png_create_info_struct(png);
  if(info == NULL){
    printf("png_crete_info_struct error!\n");
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(fi);
    return -1;
  }

  png_init_io(png, fi);
  png_set_sig_bytes(png, readSize);
  png_read_png(png, info, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

  width = png_get_image_width(png, info);
  height = png_get_image_height(png, info);

  datap = png_get_rows(png, info);

  type = png_get_color_type(png, info);
  /* とりあえずRGB or RGBAだけ対応 */
  if(type != PNG_COLOR_TYPE_RGB && type != PNG_COLOR_TYPE_RGB_ALPHA){
    printf("color type is not RGB or RGBA\n");
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fi);
    return -1;
  }

  bitmapData->width = width;
  bitmapData->height = height;
  if(type == PNG_COLOR_TYPE_RGB) {
    bitmapData->ch = 3;
  } else if(type == PNG_COLOR_TYPE_RGBA) {
    bitmapData->ch = 4;
  }
  printf("width = %d, height = %d, ch = %d\n", bitmapData->width, bitmapData->height, bitmapData->ch);

  bitmapData->data =
    (unsigned char*)malloc(sizeof(unsigned char) * bitmapData->width * bitmapData->height * bitmapData->ch);
  if(bitmapData->data == NULL){
    printf("data malloc error\n");
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fi);
    return -1;
  }

  for(j = 0; j < bitmapData->height; j++){
    memcpy(bitmapData->data + j * bitmapData->width * bitmapData->ch, datap[j], bitmapData->width * bitmapData->ch);
  }

  png_destroy_read_struct(&png, &info, NULL);
  fclose(fi);

  return 0;
}

int32_t SPNATIVE__Cotton__call_win_main(SPVM_ENV* env, SPVM_VALUE* args) {
  (void)env;
  (void)args;
  
  HINSTANCE hInst = GetModuleHandle(NULL) ;
  
  WinMain(hInst, NULL, NULL, SW_SHOWNORMAL);

  return SPVM_SUCCESS;
}
