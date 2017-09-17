//автор(с): Кудуштеев Алексей
#include "window.h"
#include "util.h"
#define CHK_YES    0
#define CHK_CPU    1
#define CHK_USER   3
#define CHK_STOP   5

static int  checkers_user_adjacent(const checkers_t* user, const checkers_t* cpu);
static int  checkers_user_test(const checkers_t* user, const checkers_t* cpu, int cur_row, int cur_col, int to_dx, int to_dy);
static int  user_attack_king(checkers_t* user, checkers_t* cpu, checker_t* pcur, int row2, int col2, int dir_x, int dir_y);
static int  is_valid_field(int row, int col);
static int  cpu_not_warning(char field[][FIELD_SIZE], int row, int col);
static void field_fill(char field[][FIELD_SIZE], checkers_t* cpu, checkers_t* user);
static int  cpu_attack_kill(checkers_t* user, checkers_t* cpu, checker_t* cur);
static int  cpu_pawn_to_king(checkers_t* cpu, const checkers_t* user);
static int  user_is_lock(const checkers_t* user, const checkers_t* cpu);
static int  king_trace_attack(const checkers_t* user, const checkers_t* cpu, const checker_t* pcur, int dir_x, int dir_y);
static int  is_none(const checkers_t* cpu, const checkers_t* user, int dy, int dx);
static int  is_user(const checkers_t* user, int dy, int dx);
static int  is_user_at(char field[][FIELD_SIZE], int row, int col);

const  int  g_offsize[2][2] = {{164, 205}, {82, 123}};
static int  g_sorted = 0;
extern polyline_t g_poly;


//инициализация шашек
void checkers_init(checkers_t* chk, int color, int cpu){
	int i, j;
	chk->cnt   = 0;
	chk->color = color;

	if(cpu){ // заполнение шашек компьютера
		for(i = 0; i < 3; ++i){//3
			for(j = (int)!(i & 1); j < FIELD_SIZE; j += 2){
				chk->arr[chk->cnt].row  = i;
				chk->arr[chk->cnt].col  = j;
				chk->arr[chk->cnt].type = CHECKER_PAWN;
				++(chk->cnt);
			}
		}
	} else { // заполнение шашек пользователя
		for(i = FIELD_SIZE - 3; i < FIELD_SIZE; ++i){
			for(j = (int)!(i & 1); j < FIELD_SIZE; j += 2){
				chk->arr[chk->cnt].row  = i;
				chk->arr[chk->cnt].col  = j;
				chk->arr[chk->cnt].type = CHECKER_PAWN;
				++(chk->cnt);
			}
		}
	}
	g_sorted = rand() % 3;
}


//поиск шашки
int checkers_indexOf(const checkers_t* chk, int row, int col){
	const checker_t* p = &chk->arr[0];
	const checker_t* e = &chk->arr[chk->cnt];
	for(; p != e; ++p){
		if((p->row == row) && (p->col == col))
			return (int)(INT_PTR)(p - &chk->arr[0]);
	}
	return -1;
}


//поиск шашки
int checkers_find(const checkers_t* chk, int row, int col){
	const checker_t* p = &chk->arr[0];
	const checker_t* e = &chk->arr[chk->cnt];
	for(; p != e; ++p){
		if((p->row == row) && (p->col == col))
			return 1;
	}
	return 0;
}


//удаление шашки по-индексу
void checkers_remove(checkers_t* chk, int index){
	int i;
	if(index >= chk->cnt)
		return;

	--(chk->cnt);
	for(i = index; i < chk->cnt; ++i)
		chk->arr[i] = chk->arr[i + 1];
}

void checkers_delete(checkers_t* chk, int row, int col){
	int index = checkers_indexOf(chk, row, col);
	if(index != -1)
		checkers_remove(chk, index);
}


static void checkers_sort(checkers_t* chk){
	checker_t t;
	int i, j;
	for(i = 1; i < chk->cnt; ++i){
		t = chk->arr[i];
		for(j = i - 1; (j >= 0) && (chk->arr[j].row < t.row); --j)
			chk->arr[j + 1] = chk->arr[j];
		chk->arr[j + 1] = t;
	}
}


//проверка всех шашек на сруб
static int checkers_user_adjacent(const checkers_t* user, const checkers_t* cpu){
	const checker_t* p, *e;
	int i, dx, dy, dx1, dy1;

	const char dirs[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};

	e = &user->arr[user->cnt];
	for(p = &user->arr[0]; p != e; ++p){

		if(p->type == CHECKER_PAWN){ //проверка шашки

			for(i = 0; i < 4; ++i){
				dx = p->col + dirs[i][0];
				dy = p->row + dirs[i][1];
				if(! is_valid_field(dy, dx))
					continue;
				if(! checkers_find(cpu, dy, dx))
					continue;
				
				dx1 = dx + dirs[i][0];
				dy1 = dy + dirs[i][1];
				if(! is_valid_field(dy1, dx1))
					continue;

				if(! checkers_find(cpu, dy1, dx1)){
					if(! checkers_find(user, dy1, dx1))
						return 1;
				}
			}

		} else if(p->type == CHECKER_KING){ // проверка дамки
			
			for(i = 0; i < 4; ++i){
				if(king_trace_attack(user, cpu, p, dirs[i][0], dirs[i][1]))
					return 1;
			}
		}
	}
	return 0;
}


//проверка указанной шашки на сруб
static int checkers_user_test(const checkers_t* user, const checkers_t* cpu, int cur_row, int cur_col, int to_dx, int to_dy){
	cur_col += to_dx;
	cur_row += to_dy;
	if(! is_valid_field(cur_row, cur_col))
		return 0;
	if(checkers_find(user, cur_row, cur_col) || ! checkers_find(cpu, cur_row, cur_col))
		return 0;

	cur_col += to_dx;
	cur_row += to_dy;
	if(! is_valid_field(cur_row, cur_col))
		return 0;
	return (! checkers_find(user, cur_row, cur_col) && ! checkers_find(cpu, cur_row, cur_col));
}


//проверка шашки вокруг себя для сруба
int checkers_is_look(const checkers_t* user, const checkers_t* cpu, int row, int col){
	int i, dx, dy;
	const char dirs[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};

	for(i = 0; i < 4; ++i){
		dx = col + dirs[i][0];
		dy = row + dirs[i][1];
		if(! is_valid_field(dy, dx))
			continue;
		if(checkers_find(cpu, dy, dx)){
			dx += dirs[i][0];
			dy += dirs[i][1];
			if(! is_valid_field(dy, dx))
				continue;
			if(! checkers_find(user, dy, dx) && ! checkers_find(cpu, dy, dx))
				return 1;
		}
	}
	return 0;
}


//атака или просто ход
static int user_attack_king(checkers_t* user, checkers_t* cpu, checker_t* pcur, int row2, int col2, int dir_x, int dir_y){
	int r, c, x, y, hod = 0;
	if(checkers_find(cpu, row2, col2) || checkers_find(user, row2, col2))
		return 0;

	c = pcur->col;
	r = pcur->row;
	while((c != col2) && (r != row2)){
		c += dir_x;
		r += dir_y;
		if(! is_valid_field(r, c) || checkers_find(user, r, c))
			return 0;
		else if(checkers_find(cpu, r, c)){
			if(++hod > 1)
				return 0;
			x = c;
			y = r;
		}
	}

	if(hod){ // значит есть под сруб
		pcur->col = col2;
		pcur->row = row2;
		checkers_delete(cpu, y, x);
		return CHECKER_KILL;
	}
	return CHECKER_MOVE;
}


//проверка на победителя
int test_finish(checkers_t* user, checkers_t* cpu){
	if(! user->cnt)
		return VICTORY_CPU;
	else if(! cpu->cnt)
		return VICTORY_USER;

	//проверить в сортире ли пользовательские шашки?
	return user_is_lock(user, cpu) ? VICTORY_CPU : 0;
}


//***************************************************************************************************************


//ход компьютера
int cpu_attack(checkers_t* cpu, checkers_t* user){
	char       field[FIELD_SIZE][FIELD_SIZE];
	checker_t* p, *e, *m;
	int dx, dy, i, r, c;

	const char dirs[4][2] = {{-1,1},{1,1},{1,-1},{-1,-1}};

	if(g_sorted == 1)
		checkers_sort(cpu);
	else if(g_sorted == 2){
		if(rand() % 2)
			checkers_sort(cpu);
	}

	//проверка на сруб
	e = &cpu->arr[cpu->cnt];
	for(p = &cpu->arr[0]; p != e; ++p){
		if(cpu_attack_kill(user, cpu, p)){
			MessageBeep((UINT)-1);
			return CPU_KILL;
		}
	}

	//попытка проскочить в дамки или другая стратегия "бытия"
	if(cpu_pawn_to_king(cpu, user))
		return CPU_MOVE;

	field_fill(field, cpu, user);

	//ход компьютера
	r = c = -1;
	for(m = p = &cpu->arr[0]; p != e; ++p){
		if(p->type == CHECKER_PAWN){ // ход пешки только вниз only down

			for(i = 0; i < 2; ++i){
				dx = p->col + dirs[i][0];
				dy = p->row + dirs[i][1];
				if(! is_valid_field(dy, dx))
					continue;

				if(field[dy][dx] == CHK_YES){
					r = dy;
					c = dx;
					m = p;
					if(is_user_at(field, dy, dx)){
						p->row = dy;
						p->col = dx;
						if(p->row >= FIELD_SIZE - 1){
							p->type = CHECKER_KING;
							MessageBeep(MB_ICONQUESTION);
						}
						return CPU_MOVE;						
					}
				}
			}

		} else if(p->type == CHECKER_KING){// ход дамки вниз/вверх

			for(i = 0; i < 4; ++i){
				dx = p->col + dirs[i][0];
				dy = p->row + dirs[i][1];
				if(! is_valid_field(dy, dx) || (field[dy][dx] != CHK_YES))
					continue;

				if(is_user_at(field, dy, dx)){
					p->row = dy;
					p->col = dx;
					return CPU_MOVE;
				} else {
					c = dx;
					r = dy;
					
					do {
						dx += dirs[i][0];
						dy += dirs[i][1];
						if(! is_valid_field(dy, dx))
							break;
						else if(field[dy][dx] == CHK_YES){
							if(is_user_at(field, dy, dx)){
								p->row = dy;
								p->col = dx;
								return CPU_MOVE;
							}
							c = dx;
							r = dy;
						} else
							break;
					} while(1);

					p->row = r;
					p->col = c;
					return CPU_MOVE;
				}
			}
		}
	}

	if((r != -1) && (m->type == CHECKER_PAWN)){
		m->col = c;
		m->row = r;
		if(m->row >= FIELD_SIZE - 1){
			m->type = CHECKER_KING;
			MessageBeep(MB_ICONQUESTION);
		}
		return CPU_MOVE;
	}
	return CPU_ERROR;
}



//ход или сруб пользователя
int user_attack(int sel_index, checkers_t* user, checkers_t* cpu, int row, int col){
	int dx, dy, mx, my, ret;
	checker_t*  cur = &user->arr[sel_index];

	if(checkers_indexOf(cpu, row, col) != -1)
		return 0;

	dx = col - cur->col;
	dy = row - cur->row;
	mx = abs(dx);
	my = abs(dy);
	if((dx == 0) || (dy == 0) || (mx != my)){ //ошибки от "дурака"
		MessageBeep(MB_ICONEXCLAMATION);
		return 0;
	}
	dx /= mx;
	dy /= my;

	if(cur->type == CHECKER_PAWN){ // пешка

		if((mx == 1) && (my == 1)){ // ходьба
			if(row >= cur->row){ // попытка ходьбы назад
				MessageBeep(MB_ICONEXCLAMATION);
				return 0;
			}

			//здесь нужно проверить на сруб вражеской шашки
			if(checkers_user_adjacent(user, cpu))
				return CHECKER_DEAD;

			cur->col += dx;
			cur->row += dy;
			if(cur->row <= 0){ //если дошли до конца, значит превращение в дамку
				cur->type = CHECKER_KING;
				MessageBeep(MB_ICONQUESTION);
			} else
				MessageBeep(MB_ICONINFORMATION);

			return CHECKER_MOVE;
		} else if((mx == 2) && (my == 2)){ // сруб
	
			//проверить на сруб
			if(! checkers_user_test(user, cpu, cur->row, cur->col, dx, dy)){
				MessageBeep(MB_ICONEXCLAMATION);
				return 0;
			}
			cur->col += dx;
			cur->row += dy;
			checkers_delete(cpu, cur->row, cur->col);
			cur->col  = col;
			cur->row  = row;
			MessageBeep((UINT)-1);

			if(cur->row <= 0){ //если дошли до конца, значит превращение в дамку
				cur->type = CHECKER_KING;
				MessageBeep(MB_ICONQUESTION);
			}
			return CHECKER_KILL;
		}

	} else if(cur->type == CHECKER_KING){ // дамка

		ret = user_attack_king(user, cpu, cur, row, col, dx, dy);
		switch(ret){
		case CHECKER_MOVE:
			
			//здесь нужно проверить на сруб вражеской шашки
			if(checkers_user_adjacent(user, cpu))
				return CHECKER_DEAD;
			
			cur->col = col;
			cur->row = row;
			break;
		case CHECKER_KILL:
			MessageBeep((UINT)-1);
			break;
		default:
			MessageBeep(MB_ICONEXCLAMATION);
			break;
		}
		return ret;
	}
	return 0;
}



//--------------------------------------------------------------------------------------------------------------



static void field_fill(char field[][FIELD_SIZE], checkers_t* cpu, checkers_t* user){
	checker_t* p, *e;
	int i, j;
	//моделирование по-матрице
	for(i = 0; i < FIELD_SIZE; ++i){
		for(j = 0; j < FIELD_SIZE; ++j)
			field[i][j] = CHK_YES;
	}
	e = &cpu->arr[cpu->cnt];
	for(p = &cpu->arr[0]; p != e; ++p)
		field[p->row][p->col] = CHK_CPU;

	e = &user->arr[user->cnt];
	for(p = &user->arr[0]; p != e; ++p)
		field[p->row][p->col] = CHK_USER;
}


//проверка вокруг себя на опасность
static int cpu_not_warning(char field[][FIELD_SIZE], int row, int col){
	int i, cnt, dx, dy;
	const char dirs[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};

	for(cnt = i = 0; i < 4; ++i){
		dx = col + dirs[i][0];
		dy = row + dirs[i][1];
		if(! is_valid_field(dy, dx)){
			++cnt;
			continue;
		} else if(field[dy][dx] == CHK_YES)
			++cnt;
	}
	return cnt;
}


//просмотр вокруг себя вражсеких шашек
static int is_user_at(char field[][FIELD_SIZE], int row, int col){
	int i, dx, dy;
	const char dirs[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};

	for(i = 0; i < 4; ++i){
		dx = col + dirs[i][0];
		dy = row + dirs[i][1];
		if(! is_valid_field(dy, dx))
			continue;
		else if(field[dy][dx] == CHK_USER)
			return 0;
	}
	return 1;
}


//путь срубывание шашкой
static int cpu_attack_kill(checkers_t* user, checkers_t* cpu, checker_t* cur){
	coord_t stk[24], p;
	int     dx, dy, dx1, dy1, x, y, i, j, q, wq = 0, n = 0;
	char    field[FIELD_SIZE][FIELD_SIZE] = {{0}};

	const char dirs[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};
	
	field_fill(field, cpu, user);

	if(cur->type == CHECKER_PAWN){ // стратегия срубаний пешкой
		q = 1;
		stk[0].col = cur->col;
		stk[0].row = cur->row;

		while(q > 0){
			p = stk[q - 1];
			--q;

			for(i = 0; i < 4; ++i){
				dx = p.col + dirs[i][0];
				dy = p.row + dirs[i][1];
				if(! is_valid_field(dy, dx))
					continue;

				if(field[dy][dx] == CHK_USER){
					dx1 = dx + dirs[i][0];
					dy1 = dy + dirs[i][1];
					if(! is_valid_field(dy1, dx1))
						continue;

					if(field[dy1][dx1] == CHK_YES){ // ход свободный
						n = 1;
						stk[q].col = dx1;
						stk[q].row = dy1;
						++q;
						checkers_delete(user, dy, dx);

						polyline_add(&g_poly, cur->col, cur->row);
						polyline_add(&g_poly, dx1, dy1);						

						cur->col = dx1;
						cur->row = dy1;

						field[p.row][p.col] = CHK_YES;
						field[dy][dx]       = CHK_YES;
						field[dy1][dx1]     = CHK_CPU;

						if(cur->row >= (FIELD_SIZE - 1)){
							field[p.row][p.col] = CHK_STOP;
							cur->type = CHECKER_KING;
							MessageBeep(MB_ICONQUESTION);
							goto king_next;
						}
						break;
					}
				}
			}
		}

	} else if(cur->type == CHECKER_KING){ // стратегия срубаний дамкой
king_next:
		q = 1;
		stk[0].col = cur->col;
		stk[0].row = cur->row;
	
		while(q > 0){
			p = stk[q - 1];
			--q;
	
			for(i = 0; i < 4; ++i){
				dx = p.col + dirs[i][0];
				dy = p.row + dirs[i][1];
				while(is_valid_field(dy, dx) && ((field[dy][dx] & 1) == 0)){
					dx += dirs[i][0];
					dy += dirs[i][1];
				}

				if(is_valid_field(dy, dx) && (field[dy][dx] == CHK_USER)){

					dx1 = dx + dirs[i][0];
					dy1 = dy + dirs[i][1];
					if(is_valid_field(dy1, dx1) && (field[dy1][dx1] == CHK_YES)){
						n  = 1;
						wq = 1;
						field[p.row][p.col] = CHK_STOP;
						field[dy][dx]       = CHK_YES;
						field[dy1][dx1]     = CHK_CPU;
						checkers_delete(user, dy, dx);

						polyline_add(&g_poly, cur->col, cur->row);
						polyline_add(&g_poly, dx1, dy1);

						cur->col = dx1;
						cur->row = dy1;

						//теперь надо найти позицию для сруба
						for(j = 0; j < 4; ++j){
							dx = dx1;
							dy = dy1;
							do {
								dx += dirs[j][0];
								dy += dirs[j][1];
							} while(is_valid_field(dy, dx) && ((field[dy][dx] & 1) == 0));

							if(is_valid_field(dy, dx) && (field[dy][dx] == CHK_USER)){

								x = dx + dirs[j][0];
								y = dy + dirs[j][1];
								if(is_valid_field(y, x) && (field[y][x] == CHK_YES)){

									field[cur->row][cur->col] = CHK_STOP;
									field[dy][dx]             = CHK_YES;
									field[y][x]               = CHK_CPU;

									checkers_delete(user, dy, dx);

									polyline_add(&g_poly, x, y);

									cur->col   = x;
									cur->row   = y;
									stk[q].col = x;
									stk[q].row = y;
									++q;
									wq = 0;
									break;
								}
							}
						}//конец for


						//если цель не была найдена пройдём дальше до безопасного места
						if(wq){
							wq = 0;
							dx = dx1;
							dy = dy1;
							do {
								dx1 += dirs[i][0];
								dy1 += dirs[i][1];
								if(! is_valid_field(dy1, dx1) || (field[dy1][dx1] != CHK_YES))
									break;
								else if(cpu_not_warning(field, dy1, dx1) >= 4){
									field[cur->row][cur->col] = CHK_YES;
									field[dy1][dx1] = CHK_CPU;

									polyline_add(&g_poly, dx1, dy1);

									cur->row = dy1;
									cur->col = dx1;
									break;
								}
							} while(1);
						}

						goto next_break;
					}
				}
			}
next_break:;
		}
	}
	return n;
}


//попытка проскочить в дамки если остался один шаг, ну или уйти из под удара
static int cpu_pawn_to_king(checkers_t* cpu, const checkers_t* user){
	int i, j, dx, dy;
	checker_t* p, *e;

	const char dirs[2][2]  = {{-1,1},{1,1}};

	//защита своей пешки или дамки от сруба
	e = &cpu->arr[cpu->cnt];
	for(p = &cpu->arr[0]; p != e; ++p){
		j  = -1;
		dx = p->col + 1;
		dy = p->row + 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row - 1, p->col - 1)){
			// значит под угрозой сруба
			dx = p->col - 1;
			dy = p->row - 1;
			if((j = checkers_indexOf(cpu, dy - 1, dx - 1)) != -1)
				goto good;
			if((j = checkers_indexOf(cpu, dy - 1, dx + 1)) != -1)
				goto good;
		}

		dx = p->col - 1;
		dy = p->row + 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row - 1, p->col + 1)){
			dx = p->col + 1;
			dy = p->row - 1;
			if((j = checkers_indexOf(cpu, dy - 1, dx + 1)) != -1)
				goto good;
			if((j = checkers_indexOf(cpu, dy - 1, dx - 1)) != -1)
				goto good;
		}

		dx = p->col + 1;
		dy = p->row - 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row + 1, p->col - 1)){
			dx = p->col - 1;
			dy = p->row + 1;
			if((j = checkers_indexOf(cpu, dy - 1, dx - 1)) != -1)
				goto good;
		}

		dx = p->col - 1;
		dy = p->row - 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row + 1, p->col + 1)){
			dx = p->col + 1;
			dy = p->row + 1;
			if((j = checkers_indexOf(cpu, dy - 1, dx + 1)) != -1)
				goto good;
		}

good:
		if(j != -1){
			cpu->arr[j].col = dx;
			cpu->arr[j].row = dy;			
			return 1;
		}
	}

	//попытка проскочить в дамки
	for(p = &cpu->arr[0]; p != e; ++p){
		if(p->row == (FIELD_SIZE - 2)){
			for(i = 0; i < 2; ++i){
				dx = p->col + dirs[i][0];
				dy = p->row + dirs[i][1];
				if(is_none(user, cpu, dy, dx)){
					if((j = checkers_indexOf(cpu, p->row, p->col)) != -1){
						cpu->arr[j].col  = dx;
						cpu->arr[j].row  = dy;
						cpu->arr[j].type = CHECKER_KING;
						return 1;
					}
				}
			}
		}
	}

	//уход от сруба
	for(p = &cpu->arr[0]; p != e; ++p){
		i  = 0;
		dx = p->col - 1;
		dy = p->row + 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row - 1, p->col + 1)){
			dx = p->col + 1;
			dy = p->row + 1;
			if(is_none(cpu, user, dy, dx) && ! is_user(user, dy + 1, dx + 1)){
				i = 1;
				goto jump;
			}
		}

		dx = p->col + 1;
		dy = p->row + 1;
		if(is_user(user, dy, dx) && is_none(cpu, user, p->row - 1, p->col - 1)){
			dx = p->col - 1;
			dy = p->row + 1;
			if(is_none(cpu, user, dy, dx) && ! is_user(user, dy + 1, dx - 1)){
				i = 1;
				goto jump;
			}
		}

		if(is_user(user, p->row - 1, p->col - 1) || is_user(user, p->row - 1, p->col + 1)){
			dx = p->col + 1;
			dy = p->row + 1;
			if(is_none(cpu, user, dy, dx) && ! is_user(user, dy + 1, dx + 1)){
				i = 1;
				goto jump;
			}
			dx = p->col - 1;
			dy = p->row + 1;
			if(is_none(cpu, user, dy, dx) && ! is_user(user, dy + 1, dx - 1)){
				i = 1;
				goto jump;
			}
		}

jump:
		if(i){
			if((j = checkers_indexOf(cpu, p->row, p->col)) != -1){
				cpu->arr[j].col = dx;
				cpu->arr[j].row = dy;
				if(dy >= (FIELD_SIZE - 1))
					cpu->arr[j].type = CHECKER_KING;
				return 1;
			}
		}
	}

	//попытка занять самый крайний левый или правый угол доски
	for(p = &cpu->arr[0]; p != e; ++p){
		for(i = 0; i < 2; ++i){
			dx = p->col + dirs[i][0];
			dy = p->row + dirs[i][1];
			if(! is_valid_field(dy, dx))
				continue;
			else if(!checkers_find(cpu, dy, dx) && !checkers_find(user, dy, dx)){
				if(! is_valid_field(dy + dirs[i][1], dx + dirs[i][0])){
					j = checkers_indexOf(cpu, p->row, p->col);
					if(j != -1){
						cpu->arr[j].col = dx;
						cpu->arr[j].row = dy;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}


//проверяем путь к цели
static int king_trace_attack(const checkers_t* user, const checkers_t* cpu, const checker_t* pcur, int dir_x, int dir_y){
	int c = pcur->col;
	int r = pcur->row;
	while(1){
		c += dir_x;
		r += dir_y;
		if(! is_valid_field(r, c) || checkers_find(user, r, c))
			return 0;
		else if(checkers_find(cpu, r, c)){
			c += dir_x;
			r += dir_y;
			if(! is_valid_field(r, c))
				break;
			if(! checkers_find(user, r, c) && ! checkers_find(cpu, r, c))
				return 1;
		}
	}
	return 0;
}


//проверить заблокированы ли шашки в сортире?
static int user_is_lock(const checkers_t* user, const checkers_t* cpu){
	const checker_t* p, *e;
	int i, dx, dy, m, k, n = 0;
	
	const char dirs[4][2] = {{1,-1},{-1,-1},{1,1},{-1,1}};

	e = &user->arr[user->cnt];
	for(p = &user->arr[0]; p != e; ++p){
		m = (p->type == CHECKER_PAWN) ? 2 : 4;
		k = 0;
		for(i = 0; i < m; ++i){
			dx = p->col + dirs[i][0];
			dy = p->row + dirs[i][1];
			if(! is_valid_field(dy, dx))
				++k;
			else if(checkers_find(user, dy, dx) || checkers_find(cpu, dy, dx)){
				dx += dirs[i][0];
				dy += dirs[i][1];
				if(! is_valid_field(dy, dx) || checkers_find(user, dy, dx) || checkers_find(cpu, dy, dx))
					++k;
			}
		}

		if(p->type == CHECKER_PAWN)
			n += (int)(k >= 2);
		else
			n += (int)(k >= 4);
	}
	return (! checkers_user_adjacent(user, cpu) && (n >= user->cnt));
}


static int is_valid_field(int row, int col){
	return ((row >= 0) && (col >= 0) && (row < FIELD_SIZE) && (col < FIELD_SIZE));
}

static int is_none(const checkers_t* cpu, const checkers_t* user, int dy, int dx){
	if(! is_valid_field(dy, dx))
		return 0;
	return (! checkers_find(user, dy, dx) && ! checkers_find(cpu, dy, dx));
}

static int is_user(const checkers_t* user, int dy, int dx){
	if(! is_valid_field(dy, dx))
		return 0;
	return checkers_find(user, dy, dx); 
}