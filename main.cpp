
#include "window.h"
#include "util.h"
#include "resource.h"

#ifdef __BORLANDC__
#pragma resource "resource.res"
#endif

#define CHK_NEXT      1
#define CHK_GAME_OVER 2
static void draw_field(void);
static void coord_to_index(int px, int py, int* row, int* col);
static int  index_is_valid(int row, int col);
static void call_game_victory(int kto);
static void change_size(int lvl);
static void start_game(HWND hwnd);

static HWND       g_hwnd  = NULL;
static HWND       g_mwnd  = NULL;
static HPEN       g_spen  = NULL;
static int        g_size  = 0;
static int        g_cell  = 0;
static int        g_next  = 0;
static int        g_imsg  = 0;
static int        g_index = -1;
static bitmap_t   g_canvas;
static bitmap_t   g_tiles;
static checkers_t g_user;
static checkers_t g_cpu;
static int        g_hov_user;
static int        g_sel_user;
polyline_t        g_poly;



void OnCreate(void){
	HINSTANCE inst = (HINSTANCE)GetModuleHandle(NULL);

	srand((unsigned int)time(NULL));

	bitmap_create(g_hwnd, g_size, g_size, &g_canvas);
	bitmap_resource(g_hwnd, inst, IDB_BITMAP1, &g_tiles);

	SetStretchBltMode(g_canvas.hdc, HALFTONE);
	g_spen = CreatePen(PS_SOLID, 2, RGB(0, 0x77, 0xFF));
	
	g_mwnd = g_createMsgDlg(inst, 210, 80);

	polyline_init(&g_poly, RGB(0xFF, 0x22, 0x11));
}
	


//--------------------------------------------------------------------------------------------------------------


void OnInitialize(HWND hwnd){
	start_game(hwnd);
}


//--------------------------------------------------------------------------------------------------------------


//команды меню
void OnCommand(WPARAM wParam){
	switch(LOWORD(wParam)){
	case MENU_NEWGAME:
		start_game(g_hwnd);
		break;
	case MENU_CHK_WHITE:
		g_user.color = CHECKER_WHITE;
		g_cpu.color  = CHECKER_BLACK;
		InvalidateRect(g_hwnd, NULL, TRUE);
		break;
	case MENU_CHK_BLACK:
		g_user.color = CHECKER_BLACK;
		g_cpu.color  = CHECKER_WHITE;
		InvalidateRect(g_hwnd, NULL, TRUE);
		break;
	case MENU_FIELD_MIN:
		change_size(1);
		break;
	case MENU_FIELD_AVG:
		change_size(2);
		break;
	case MENU_FIELD_MAX:
		change_size(3);
		break;
	case MENU_QUIT:
		DestroyWindow(g_hwnd);
		break;
	}
}

	
//--------------------------------------------------------------------------------------------------------------


void OnMouseDown(WPARAM wParam, LPARAM lParam){
	int row, col, index, ret;
	int x = (int)LOWORD(lParam);
	int y = (int)HIWORD(lParam);
	coord_to_index(x, y, &row, &col);

	if(! index_is_valid(row, col) || (g_next == CHK_GAME_OVER))
		return;

	index = checkers_indexOf(&g_user, row, col);
	if((g_sel_user != -1) && (index == -1)){ // значит выбор сделан для хода и так далее
		
		switch( user_attack(g_sel_user, &g_user, &g_cpu, row, col) ){
		case CHECKER_MOVE:

			ret = cpu_attack(&g_cpu, &g_user);
			if(ret == CPU_KILL)
				g_sel_user = g_hov_user = -1;
			else if(ret == CPU_ERROR){
				call_game_victory(VICTORY_USER);
				InvalidateRect(g_hwnd, NULL, TRUE);
				return;
			}

			if((ret = test_finish(&g_user, &g_cpu)) != 0)
				call_game_victory(ret);

			g_next = 0;
			InvalidateRect(g_hwnd, NULL, TRUE);
			break;
		case CHECKER_KILL:

			if(checkers_is_look(&g_user, &g_cpu, row, col)){
				g_next = CHK_NEXT;
			} else {
				ret = cpu_attack(&g_cpu, &g_user);
				if(ret == CPU_KILL)
					g_sel_user = g_hov_user = -1;
				else if(ret == CPU_ERROR){
					call_game_victory(VICTORY_USER);
					InvalidateRect(g_hwnd, NULL, TRUE);
					return;
				}
				g_next = 0;
			}

			if((ret = test_finish(&g_user, &g_cpu)) != 0)
				call_game_victory(ret);

			InvalidateRect(g_hwnd, NULL, TRUE);
			break;
		case CHECKER_DEAD:
			show_dialog(g_mwnd, 10, 10, TEXT("У Вас есть шашка под сруб !"), RGB(0xFF, 0xFF, 0), 1500);
			break;
		}

	} else { //выбор пешки для хода

		if(g_next != CHK_NEXT){
			if(index != g_sel_user)
				InvalidateRect(g_hwnd, NULL, TRUE);
			g_sel_user = index;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------


void OnMouseMove(LPARAM lParam){
	int row, col, index;
	int x = (int)LOWORD(lParam);
	int y = (int)HIWORD(lParam);

	coord_to_index(x, y, &row, &col);
	if(! index_is_valid(row, col) || (g_next == CHK_GAME_OVER))
		return;

	//курсор находится ли над шашкой?
	index = checkers_indexOf(&g_user, row, col);
	if((index != -1) && (index != g_sel_user)){
		if(g_hov_user != g_index){
			g_hov_user = index;
			InvalidateRect(g_hwnd, NULL, TRUE);
		}
		g_index = index;
	} else {
		if(g_hov_user != -1)
			InvalidateRect(g_hwnd, NULL, TRUE);
		g_hov_user = -1;
	}
}


//--------------------------------------------------------------------------------------------------------------


void OnPaint(HDC hDC){
	draw_field();
	BitBlt(hDC, 0, 0, g_size, g_size, g_canvas.hdc, 0, 0, SRCCOPY);
}


//--------------------------------------------------------------------------------------------------------------


void OnDestroy(void){
	bitmap_free(&g_canvas);
	bitmap_free(&g_tiles);
	polyline_free(&g_poly);

	if(g_spen != NULL)
		DeleteObject(g_spen);
	g_spen = NULL;

	if(g_mwnd != NULL)
		DestroyWindow(g_mwnd);
	g_mwnd = NULL;
}


//--------------------------------------------------------------------------------------------------------------



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	MSG msg;

	g_size = GetSystemMetrics(SM_CYSCREEN) * 42 / 100;
	g_size = g_size / FIELD_SIZE * FIELD_SIZE;
	g_cell = g_size / FIELD_SIZE;
	g_hwnd = g_createWindow(hInstance, g_size, TEXT("Русские шашки"), TEXT("Checkers"), IDI_ICON1);
	if(g_hwnd == NULL)
		return 1;

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


//--------------------------------------------------------------------------------------------------------------

//рисование игрового поля
static void draw_field(void){
	const checker_t* p, *e;
	HGDIOBJ old, pen;
	int     i, j, xor;

	//рисование доски
	for(j = 0; j < FIELD_SIZE; ++j){
		xor = j & 1;
		for(i = 0; i < FIELD_SIZE; ++i){
			StretchBlt(g_canvas.hdc, i*g_cell, j*g_cell, g_cell, g_cell, 
			           g_tiles.hdc,  xor * g_tiles.height, 0, 
			           g_tiles.height, g_tiles.height, SRCCOPY);
			xor ^= 1;
		}
	}

	//вывод компьютерных шашек
	e = &g_cpu.arr[g_cpu.cnt];
	for(p = &g_cpu.arr[0]; p != e; ++p){
		StretchBlt(g_canvas.hdc, p->col*g_cell, p->row*g_cell, g_cell, g_cell, 
		           g_tiles.hdc,  g_offsize[g_cpu.color][p->type], 0, 
		           g_tiles.height, g_tiles.height, SRCCOPY);		
	}

	//вывод линии срубов, если есть
	polyline_draw(&g_poly, g_canvas.hdc, g_cell);

	//*************************************************************************************************

	//вывод пользовательских шашек
	e = &g_user.arr[g_user.cnt];
	for(p = &g_user.arr[0]; p != e; ++p){
		StretchBlt(g_canvas.hdc, p->col*g_cell, p->row*g_cell, g_cell, g_cell, 
		           g_tiles.hdc,  g_offsize[g_user.color][p->type], 0, 
		           g_tiles.height, g_tiles.height, SRCCOPY);		
	}

	if(g_hov_user != -1){ //подсветка-шашки курсора над шашкой
		p = &g_user.arr[g_hov_user];
		StretchBlt(g_canvas.hdc, p->col*g_cell, p->row*g_cell, g_cell, g_cell, 
		           g_tiles.hdc,  246, 0, g_tiles.height, g_tiles.height, SRCPAINT);		
	}

	if(g_sel_user != -1){//подсвечиваем выбранную шашку
		p   = &g_user.arr[g_sel_user];
		old = SelectObject(g_canvas.hdc, (HGDIOBJ)GetStockObject(NULL_BRUSH));
		pen = SelectObject(g_canvas.hdc, g_spen);

		i   = p->col * g_cell;
		j   = p->row * g_cell;
		xor = g_cell * 5 / 100 * 2;

		Ellipse(g_canvas.hdc, i + xor - xor/3, j + xor, i + g_cell - xor, j + g_cell - xor);
		SelectObject(g_canvas.hdc, old);
		SelectObject(g_canvas.hdc, pen);
	}
}


//преобразование координат курсора в кординаты игрового поля
static void coord_to_index(int px, int py, int* row, int* col){
	px   /= g_cell;
	py   /= g_cell;
	*col  = max(0, min(px, FIELD_SIZE - 1));
	*row  = max(0, min(py, FIELD_SIZE - 1));	
}


//проверяем на валидную область
static int index_is_valid(int row, int col){
	return ((!(row & 1)) && (col & 1)) || ((!(col & 1)) && (row & 1));
}


//изменение размера диалога
static void change_size(int lvl){
	switch(lvl){
	case 1:
		g_size = GetSystemMetrics(SM_CYSCREEN) * 42 / 100;
		break;
	case 2:
		g_size = GetSystemMetrics(SM_CYSCREEN) * 62 / 100;
		break;
	default:
		g_size = GetSystemMetrics(SM_CYSCREEN) * 78 / 100;
		break;
	}
	g_size = g_size / FIELD_SIZE * FIELD_SIZE;
	g_cell = g_size / FIELD_SIZE;
	resize_dialog(g_hwnd, g_size);
	bitmap_free(&g_canvas);
	bitmap_create(g_hwnd, g_size, g_size, &g_canvas);
	InvalidateRect(g_hwnd, NULL, TRUE);
}


//победитель
static void call_game_victory(int kto){
	if(kto == VICTORY_CPU){
		g_next = CHK_GAME_OVER;
		show_dialog(g_mwnd, 10, 10, TEXT("ВЫ ПРОИГРАЛИ ИГРУ!"), RGB(0xFF, 0xF, 0xF), 2200);	
		MessageBeep(MB_ICONERROR);
	} else if(kto == VICTORY_USER){
		g_next = CHK_GAME_OVER;
		show_dialog(g_mwnd, 10, 10, TEXT("Вы выиграли игру, тем самым разгромили компьютер!"), RGB(0x11, 0xFF, 0x32), 3500);	
	}
}


static void start_game(HWND hwnd){
	int uc, cc;
	get_state_color(&uc, &cc);
	checkers_init(&g_user, uc, 0);
	checkers_init(&g_cpu,  cc, 1);
	g_hov_user = g_sel_user = g_index = -1;
	g_next = 0;

	InvalidateRect(hwnd, NULL, TRUE);

	if(MessageBox(hwnd, TEXT("Вы хотите ходить первым, Да или Нет?"), TEXT("Первый ход"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		cpu_attack(&g_cpu, &g_user);

	InvalidateRect(hwnd, NULL, TRUE);
}
