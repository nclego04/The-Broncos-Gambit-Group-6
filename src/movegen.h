#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"

// Move generation functions implemented in movegen.c
void gen_pawn(const Pos *p, int from, int white, Move *moves, int *n);
void gen_knight(const Pos *p, int from, int white, Move *moves, int *n);
void gen_queen(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_bishop(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_rook(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_king(const Pos *p, int from, int white, Move *moves, int *n);
void gen_slider(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);

#endif // MOVEGEN_H
