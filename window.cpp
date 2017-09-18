#include "window.h"
#define CHK_TIMER_ID   777

#ifndef IDC_HAND
#define IDC_HAND  MAKEINTRESOURCE(32649)
#endif

static BOOL  __onLocalCmd(WPARAM wParam);
static void  setAlpha(HWND hwnd, BOOL enable, BYTE alpha);
static BOOL  addMenuItem(HMENU menu, const TCHAR* str, UINT id);
static HMENU addMenu(HMENU menu, LPCTSTR name);
static BOOL  addMenuSeparator(HMENU menu);
static void  setCheck(UINT id, BOOL show);
static BOOL  isChecked(UINT id);

static HMENU g_menu = NULL;



//обработчик сообщений
static LRESULT CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_CREATE:
		OnCreate();
		break;
	case WM_COMMAND:
		if(! __onLocalCmd(wParam))
			OnCommand(wParam);
		break;
	case WM_LBUTTONDOWN:
		OnMouseDown(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(lParam);
		break;
	case WM_ERASEBKGND:
		OnPaint((HDC)wParam);
		return 1;
	case WM_DESTROY:
		OnDestroy();

		if(g_menu != NULL)
			DestroyMenu(g_menu);
		g_menu = NULL;

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}



// функция создаёт окно...
HWND g_createWindow(HINSTANCE hInst, int size, LPCTSTR caption, LPCTSTR cname, UINT id_icon){
	HWND  hwnd;
	HMENU menu;
	RECT  rect;
	DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX);

	WNDCLASSEX wcls = {0};
	wcls.cbSize = sizeof(WNDCLASSEX);

	// проверить класс-окна, вдруг есть такой уже
	if(GetClassInfoEx(hInst, cname, &wcls))
		return NULL;

	ZeroMemory(&wcls, sizeof(wcls));
	wcls.cbSize        = sizeof(WNDCLASSEX);
	wcls.lpfnWndProc   = (WNDPROC)DlgProc;
	wcls.hInstance     = hInst;
	wcls.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(id_icon));
	wcls.hCursor       = LoadCursor(NULL, IDC_HAND);
	wcls.lpszClassName = cname;
	wcls.hIconSm       = NULL;
	wcls.hbrBackground = NULL;

	if(! RegisterClassEx(&wcls))
		return NULL;

	SetRect(&rect, 0, 0, size, size);
	AdjustWindowRectEx(&rect, style, TRUE, WS_EX_APPWINDOW);
	rect.right  -= rect.left;
	rect.bottom -= rect.top;

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, cname, caption, style, 
	                      (GetSystemMetrics(SM_CXSCREEN) - rect.right)  / 2, 
	                      (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2, 
	                      rect.right, rect.bottom, NULL, NULL, hInst, NULL);
	if(hwnd == NULL) {
		UnregisterClass(cname, hInst);
		return NULL;
	}
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//создаём меню
	g_menu = CreateMenu();
	if(g_menu == NULL)
		return hwnd;

	SetMenu(hwnd, g_menu);

	menu = addMenu(g_menu, TEXT("Игра"));
	addMenuItem(menu, TEXT("Новая игра"), MENU_NEWGAME);
	addMenuSeparator(menu);
	addMenuItem(menu, TEXT("Выход"), MENU_QUIT);

	menu = addMenu(g_menu, TEXT("Цвет"));
	addMenuItem(menu, TEXT("Играть белыми шашками"),  MENU_CHK_WHITE);
	addMenuItem(menu, TEXT("Играть чёрными шашками"), MENU_CHK_BLACK);
	setCheck(MENU_CHK_WHITE, TRUE);

	menu = addMenu(g_menu, TEXT("Размер"));
	addMenuItem(menu, TEXT("Маленькое поле"), MENU_FIELD_MIN);
	addMenuItem(menu, TEXT("Среднее поле"),   MENU_FIELD_AVG);
	addMenuItem(menu, TEXT("Большое поле"),   MENU_FIELD_MAX);
	setCheck(MENU_FIELD_MIN, TRUE);

	menu = addMenu(g_menu, TEXT("Автор"));
	addMenuItem(menu, TEXT("Завьялов & Юшкевич"), MENU_AUTHOR);

	DrawMenuBar(hwnd);

	OnInitialize(hwnd);
	return hwnd;
}


//---------------------------------------------------------------------------------------------------------

static TCHAR    g_text[128];
static COLORREF g_ctext = 0;


//обработчик сообщений
static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static HFONT font = NULL;
	PAINTSTRUCT  ps;
	HGDIOBJ      old;
	RECT         rect, crc;
	HDC          hdc;
	int          ret, h, len;

	switch(msg){
	case WM_CREATE:
		hdc  = GetDC(hwnd);
		font = create_font(hdc, TEXT("Arial"), 10);
		ReleaseDC(hwnd, hdc);
		break;
	case WM_LBUTTONDOWN:
		KillTimer(hwnd, CHK_TIMER_ID);
		ShowWindow(hwnd, SW_HIDE);
		break;
	case WM_PAINT:
		GetClientRect(hwnd, &crc);
		hdc = BeginPaint(hwnd, &ps);

		old = SelectObject(hdc, font);
		ret = SetBkMode(hdc, TRANSPARENT);

		SetTextColor(hdc, g_ctext);

		SetRect(&rect, 32, 0, crc.right - 10, crc.bottom - 10);

		len = lstrlen(g_text);
		h   = DrawText(hdc, g_text, len, &rect, DT_CENTER | DT_WORDBREAK | DT_CALCRECT);

		rect.left   = 32;
		rect.right  = crc.right - 10;
		rect.top    = (crc.bottom - h) / 2;
		rect.bottom = rect.top + h;

		DrawText(hdc, g_text, len, &rect, DT_CENTER | DT_WORDBREAK);

		SelectObject(hdc, old);
		SetBkMode(hdc, ret);

		EndPaint(hwnd, &ps);
		break;
	case WM_TIMER:

		if(LOWORD(wParam) == CHK_TIMER_ID){
			KillTimer(hwnd, CHK_TIMER_ID);
			ShowWindow(hwnd, SW_HIDE);
		}
		break;
	case WM_DESTROY:

		if(font != NULL)
			DeleteObject(font);
		font = NULL;
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


// функция создаёт окно...
HWND g_createMsgDlg(HINSTANCE hInst, int width, int height){
	HWND       hwnd;
	HRGN       rgn1, rgn2;
	POINT      dots[3];
	LPCTSTR    cptr    = TEXT("msgHelper");
	WNDCLASSEX wcls    = {0};

	wcls.cbSize = sizeof(WNDCLASSEX);
	// проверить класс-окна, вдруг есть такой уже
	if(GetClassInfoEx(hInst, cptr, &wcls))
		return NULL;

	ZeroMemory(&wcls, sizeof(wcls));
	wcls.cbSize        = sizeof(WNDCLASSEX);
	wcls.lpfnWndProc   = (WNDPROC)MsgProc;
	wcls.hInstance     = hInst;
	wcls.hIcon         = NULL;
	wcls.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcls.lpszClassName = cptr;
	wcls.hIconSm       = NULL;
	wcls.hbrBackground = CreateSolidBrush(RGB(0, 0, 0x55));

	if(! RegisterClassEx(&wcls))
		return NULL;

	hwnd = CreateWindowEx(WS_EX_TOPMOST, cptr, NULL, WS_POPUPWINDOW, 0, 0, 
	                      width, height, NULL, NULL, hInst, NULL);
	if(hwnd == NULL) {
		UnregisterClass(cptr, hInst);
		return NULL;
	}
	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	//создаём диалог неправильной формы
	dots[0].x = 0;
	dots[0].y = height / 2;
	dots[1].x = 29; 
	dots[1].y = height / 2 - 10;
	dots[2].x = 29;
	dots[2].y = height / 2 + 10;
        
	rgn1 = CreatePolygonRgn(dots, 3, WINDING);
	rgn2 = CreateRoundRectRgn(25, 0, width, height, 33, 33);
	CombineRgn(rgn2, rgn2, rgn1, RGN_OR);
	SetWindowRgn(hwnd, rgn2, TRUE);

	DeleteObject(rgn1);
	DeleteObject(rgn2);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	setAlpha(hwnd, TRUE, 170);
	return hwnd;
}



//-------------------------------------------------------------------------------------------------------------


BOOL bitmap_load(HWND hwnd, const TCHAR* filename, bitmap_t* pbmp){
	HDC    hdc;
	BITMAP inf;
	pbmp->bmp = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if(pbmp->bmp == NULL)
		return FALSE;
	hdc = GetDC(hwnd);
	pbmp->hdc = CreateCompatibleDC(hdc);
	SelectObject(pbmp->hdc, pbmp->bmp);

	if(GetObject(pbmp->bmp, sizeof(inf), (LPVOID)&inf)){
		pbmp->width  = inf.bmWidth;
		pbmp->height = inf.bmHeight;
	}
	ReleaseDC(hwnd, hdc);
	return TRUE;
}


BOOL bitmap_resource(HWND hwnd, HINSTANCE hInst, UINT id, bitmap_t* pbmp){
	HDC    hdc;
	BITMAP inf;
	pbmp->bmp = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if(pbmp->bmp == NULL)
		return FALSE;
	hdc = GetDC(hwnd);
	pbmp->hdc = CreateCompatibleDC(hdc);
	SelectObject(pbmp->hdc, pbmp->bmp);

	if(GetObject(pbmp->bmp, sizeof(inf), (LPVOID)&inf)){
		pbmp->width  = inf.bmWidth;
		pbmp->height = inf.bmHeight;
	}
	ReleaseDC(hwnd, hdc);
	return TRUE;
}



BOOL bitmap_create(HWND hwnd, int width, int height, bitmap_t* pbmp){
	HDC   hdc = GetDC(hwnd);
	pbmp->bmp = CreateCompatibleBitmap(hdc, width, height);
	if(pbmp->bmp != NULL){
		pbmp->hdc = CreateCompatibleDC(hdc);
		SelectObject(pbmp->hdc, pbmp->bmp);
		pbmp->width  = width;
		pbmp->height = height;
		PatBlt(pbmp->hdc, 0, 0, width, height, WHITENESS);
	}
	return (pbmp->bmp != NULL);
}



void bitmap_free(bitmap_t* pbmp){
	if(pbmp->bmp != NULL)
		DeleteObject(pbmp->bmp);
	pbmp->bmp = NULL;
	if(pbmp->hdc != NULL)
		DeleteDC(pbmp->hdc);
	pbmp->hdc = NULL;
	pbmp->width = pbmp->height = 0;
}


//инициализация
void polyline_init(polyline_t* p, COLORREF color){
	p->dots = NULL;
	p->cnt  = 0;
	p->mem  = 16;
	p->pen  = CreatePen(PS_SOLID, 2, color);
}


// добавление точки в линию
int polyline_add(polyline_t* p, int x, int y){
	DWORD  cnt;
	POINT* arr;

	if(p->dots == NULL){
		p->dots = (POINT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, p->mem * sizeof(POINT));
		if(p->dots == NULL)
			return 0;
	} else if((p->cnt + 1) >= p->mem){
		cnt = p->cnt + 1 + p->mem / 2;
		arr = (POINT*)HeapReAlloc(GetProcessHeap(), 0, p->dots, cnt * sizeof(POINT));
		if(arr == NULL)
			return 0;
		p->dots = arr;
		p->mem  = cnt;
	}
	p->dots[p->cnt].x = x;
	p->dots[p->cnt].y = y;
	++(p->cnt);
	return 1;
}


//вывод линии
void polyline_draw(polyline_t* p, HDC hDC, int scale){
	int     s;
	DWORD   i;
	HGDIOBJ old;
	if((p->cnt > 0) && (p->dots != NULL)){
		s = scale / 2 - 1;
		for(i = 0; i < p->cnt; ++i){
			p->dots[i].x = p->dots[i].x * scale + s;
			p->dots[i].y = p->dots[i].y * scale + s;
		}
		old = SelectObject(hDC, p->pen);
		Polyline(hDC, p->dots, p->cnt);
		SelectObject(hDC, old);
		p->cnt = 0;
	}
}


//очистка мусора
void polyline_free(polyline_t* p){
	if(p->dots != NULL)
		HeapFree(GetProcessHeap(), 0, p->dots);
	p->dots = NULL;

	if(p->pen != NULL)
		DeleteObject(p->pen);
	p->pen = NULL;

	p->cnt  = 0;
	p->mem  = 16;
}



HFONT create_font(HDC hDC, const TCHAR* fontname, int size){
	int   fch  = -MulDiv(size, GetDeviceCaps(hDC, LOGPIXELSY), 72);      
	HFONT font = CreateFont(fch, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, 
	                    OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, 
	                    DEFAULT_PITCH, fontname); 
	if(font == NULL)
		font = (HFONT)GetStockObject(SYSTEM_FONT);
	return font;
}



void  show_dialog(HWND hwnd, int x, int y, const TCHAR* stext, COLORREF ctext, DWORD msec){
	int   len;
	POINT pt;
	HWND  cur = GetActiveWindow();

	if(! IsWindowVisible(hwnd)){
		len = lstrlen(stext);
		if(len >= (int)(sizeof(g_text)/sizeof(g_text[0]) - 1))
			len = (int)(sizeof(g_text)/sizeof(g_text[0]) - 1);

		lstrcpyn(g_text, stext, len);
		g_text[len] = TEXT('\0');
		g_ctext     = ctext;

		GetCursorPos(&pt);
		pt.x += 5;
		pt.y -= 50;
		
		SetWindowPos(hwnd, HWND_TOPMOST, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		InvalidateRect(hwnd, NULL, TRUE);
		SetForegroundWindow(hwnd);
		SetTimer(hwnd, CHK_TIMER_ID, msec, NULL);

		if(cur != hwnd)
			SetFocus(cur);
	}
}


//изменение формы диалога
void resize_dialog(HWND hwnd, int size){
	RECT rc;
	SetRect(&rc, 0, 0, size, size);
	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX), TRUE, WS_EX_APPWINDOW);
	rc.right  -= rc.left;
	rc.bottom -= rc.top;
	SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOACTIVATE);
}


//получить цвет
void  get_state_color(int* color_user, int* color_cpu){
	if(isChecked(MENU_CHK_WHITE)){
		*color_user = 1;
		*color_cpu  = 0;
	} else {
		*color_user = 0;
		*color_cpu  = 1;
	}
}


//сообщения меню
static BOOL  __onLocalCmd(WPARAM wParam){
	int i;
	const UINT ids[] = { MENU_CHK_WHITE, MENU_CHK_BLACK, MENU_FIELD_MIN, MENU_FIELD_AVG, MENU_FIELD_MAX };
	const UINT id    = (UINT)LOWORD(wParam);

	switch(id){
	case MENU_CHK_WHITE:
	case MENU_CHK_BLACK:

		if(isChecked(id))
			return TRUE;

		for(i = 0; i < 2; ++i){
			if(ids[i] == id)
				setCheck(ids[i], TRUE);
			else if(isChecked(ids[i]))
				setCheck(ids[i], FALSE);
		}
		break;
	case MENU_FIELD_MIN:
	case MENU_FIELD_AVG:
	case MENU_FIELD_MAX:

		if(isChecked(id))
			return TRUE;

		for(i = 2; i < 5; ++i){
			if(ids[i] == id)
				setCheck(ids[i], TRUE);
			else if(isChecked(ids[i]))
				setCheck(ids[i], FALSE);
		}
		break;
	}
	return FALSE;
}


// установка прозрачности формы где нуль полная прозрачность, а 255 полная видимость
static void setAlpha(HWND hwnd, BOOL enable, BYTE alpha){
#if _WIN32_WINNT >= 0x0500
#ifdef LWA_ALPHA
	DWORD flags = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);

	if((! enable) && (flags & WS_EX_LAYERED))
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, flags & ~WS_EX_LAYERED);
	else {
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, flags | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
	}
	InvalidateRgn(hwnd, NULL, TRUE);
#endif
#endif
}


//добавление команды меню
static BOOL addMenuItem(HMENU menu, const TCHAR* str, UINT id){
	return AppendMenu(menu, MF_STRING, id, str);
}


static BOOL addMenuSeparator(HMENU menu){
	return AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
}


static HMENU addMenu(HMENU menu, LPCTSTR name){
	HMENU msub;
	if(menu == NULL)
		return NULL;

	msub = CreatePopupMenu();
	if(msub == NULL)
		return NULL;

	if(! AppendMenu(menu, MF_STRING | MF_POPUP, (UINT_PTR)msub, name)){
		DestroyMenu(msub);
		msub = NULL;
	}
	return msub;
}


static void setCheck(UINT id, BOOL show){
	UINT flag = (show) ? MF_CHECKED : MF_UNCHECKED;
	CheckMenuItem(g_menu, id, MF_BYCOMMAND | flag);
}

static BOOL isChecked(UINT id){
	return (GetMenuState(g_menu, id, MF_BYCOMMAND) & MF_CHECKED) ? TRUE : FALSE;
}