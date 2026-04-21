#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "engine.h"

int negamax(const Pos *p, int depth, int alpha, int beta, Move *best_move);
void search_position(const Pos *p, const char *go_cmd);
long long get_time_ms(void);

#endif // ALGORITHM_H
