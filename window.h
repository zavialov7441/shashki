//автор(с): Кудуштеев Алексей
#if ! defined(_WINDOW_KUDUSHTEEV_H_)
#define _WINDOW_KUDUSHTEEV_H_
#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif
#ifdef __BORLANDC__
#pragma warn -8004
#endif
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MENU_NEWGAME   100
#define MENU_STATUS    101
#define MENU_QUIT      102
#define MENU_CHK_WHITE 103
#define MENU_CHK_BLACK 104
#define MENU_FIELD_MIN 105
#define MENU_FIELD_AVG 106
#define MENU_FIELD_MAX 107
#define MENU_AUTHOR    108

extern HWND g_createWindow(HINSTANCE hInst, int size, LPCTSTR caption, LPCTSTR cname, UINT id_icon);
extern HWND g_createMsgDlg(HINSTANCE hInst, int width, int height); 
extern void OnCreate(void);
extern void OnInitialize(HWND hwnd);
extern void OnCommand(WPARAM wParam);
extern void OnMouseDown(WPARAM wParam, LPARAM lParam);
extern void OnMouseMove(LPARAM lParam);
extern void OnPaint(HDC hDC);
extern void OnDestroy(void);



typedef struct {
	HDC     hdc;
	HBITMAP bmp;
	int     width;
	int     height;
} bitmap_t;

extern BOOL  bitmap_load(HWND hwnd, const TCHAR* filename, bitmap_t* pbmp);
extern BOOL  bitmap_resource(HWND hwnd, HINSTANCE hInst, UINT id, bitmap_t* pbmp);
extern BOOL  bitmap_create(HWND hwnd, int width, int height, bitmap_t* pbmp);
extern void  bitmap_free(bitmap_t* pbmp);
extern HFONT create_font(HDC hDC, const TCHAR* fontname, int size);
extern void  show_dialog(HWND hwnd, int x, int y, const TCHAR* stext, COLORREF ctext, DWORD msec);
extern void  resize_dialog(HWND hwnd, int size);
extern void  get_state_color(int* color_user, int* color_cpu);



typedef struct {
	HPEN   pen;
	POINT* dots;
	DWORD  cnt;
	DWORD  mem;
} polyline_t;

extern void polyline_init(polyline_t* p, COLORREF color);
extern int  polyline_add(polyline_t* p, int x, int y);
extern void polyline_draw(polyline_t* p, HDC hDC, int scale);
extern void polyline_free(polyline_t* p);

#endif