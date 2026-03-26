#ifndef MOVEGEN_H
#define MOVEGEN_H

/**
 * @brief Represents a single chess move.
 */
typedef struct {
    int from, to;    // 0-63 1D indices representing the source and destination squares
    char promo;      // Piece character to promote to ('q', 'r', 'b', 'n'), or 0 if no promotion
} Move;

/**
 * @brief Represents the complete state of a chess board.
 */
typedef struct {
    char b[64];        // Board array: '.' for empty, 'P','N','B','R','Q','K' (White), 'p','n','b','r','q','k' (Black)
    int white_to_move; // 1 if it is White's turn to move, 0 if Black's turn
    int castling;      // Bitmask - 1: WK, 2: WQ, 4: BK, 8: BQ (Kingside/Queenside rights)
    int ep;            // En passant target square index (0-63), or -1 if no en passant capture is available
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
