#ifndef ENGINE_H
#define ENGINE_H

#include "types.h"

// Core functions implemented in engine.c
void pos_start(Pos *p);
void pos_from_fen(Pos *p, const char *fen);
int sq_index(const char *s);
int is_white_piece(char c);
void add_move(Move *moves, int *n, int from, int to, char promo);
int is_square_attacked(const Pos *p, int sq, int by_white);
Pos make_move(const Pos *p, Move m);
int legal_moves(const Pos *p, Move *out);
void apply_uci_move(Pos *p, const char *uci);
void parse_position(Pos *p, const char *line);
void index_to_sq(int idx, char out[3]);
void print_bestmove(Move m);
int in_check(const Pos *p, int white_king);
int pseudo_legal_moves(const Pos *p, Move *moves);


#endif // ENGINE_H