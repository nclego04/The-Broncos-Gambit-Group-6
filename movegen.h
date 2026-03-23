#ifndef MOVEGEN_H
#define MOVEGEN_H

typedef struct {
    int from, to;
    char promo;
} Move;

typedef struct {
    char b[64];
    int white_to_move;
    int castling; // 1: K, 2: Q, 4: k, 8: q
    int ep;       // en passant target square index, -1 if none
} Pos;

// Functions remaining in engine.c but needed in movegen.c
int is_white_piece(char c);
void add_move(Move *moves, int *n, int from, int to, char promo);
int is_square_attacked(const Pos *p, int sq, int by_white);
Pos make_move(const Pos *p, Move m);
int legal_moves(const Pos *p, Move *out);
void index_to_sq(int idx, char out[3]);
void print_bestmove(Move m);

// Move generation functions implemented in movegen.c
void gen_pawn(const Pos *p, int from, int white, Move *moves, int *n);
void gen_knight(const Pos *p, int from, int white, Move *moves, int *n);
void gen_queen(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_bishop(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_rook(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n);
void gen_king(const Pos *p, int from, int white, Move *moves, int *n);

#endif // MOVEGEN_H