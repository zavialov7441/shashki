//автор(с): Кудуштеев Алексей
#if ! defined(_RUSSIAN_CHECKERS_KUDUSHTEEV_H_)
#define _RUSSIAN_CHECKERS_KUDUSHTEEV_H_
#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
typedef __int8  int8_t;
typedef __int16 int16_t;
#else
#include <stdint.h>
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#define FIELD_SIZE    8
#define CHECKERS_MAX  12

#define CHECKER_BLACK 0
#define CHECKER_WHITE 1
#define CHECKER_PAWN  0
#define CHECKER_KING  1

#define CHECKER_MOVE  1
#define CHECKER_KILL  2 
#define CHECKER_DEAD  4

#define CPU_MOVE      1
#define CPU_KILL      2
#define CPU_ERROR     3

#define VICTORY_CPU   1
#define VICTORY_USER  2

typedef struct {
	int8_t row;
	int8_t col;
} coord_t;


typedef struct {
	int16_t type;
	int8_t  row;
	int8_t  col;
} checker_t;


typedef struct {
	checker_t arr[CHECKERS_MAX];
	int       cnt;
	int       color;
} checkers_t;

extern const int g_offsize[2][2];

extern void checkers_init(checkers_t* chk, int color, int cpu);
extern int  checkers_indexOf(const checkers_t* chk, int row, int col);
extern int  checkers_find(const checkers_t* chk, int row, int col);
extern void checkers_remove(checkers_t* chk, int index);
extern void checkers_delete(checkers_t* chk, int row, int col);
extern int  checkers_is_look(const checkers_t* user, const checkers_t* cpu, int row, int col);
extern int  cpu_attack(checkers_t* cpu, checkers_t* user);
extern int  user_attack(int sel_index, checkers_t* user, checkers_t* cpu, int row, int col);
extern int  test_finish(checkers_t* user, checkers_t* cpu);

#endif

