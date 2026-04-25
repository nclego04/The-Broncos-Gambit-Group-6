#ifndef EVALUATE_H
#define EVALUATE_H

#include "types.h"

int evaluate(const Pos *p);

// Helper functions exposed for testing
void evaluate_pawn_structure(int sq, int is_w, const int pawns[2][8], const int min_pawn_rank[2][8], const int max_pawn_rank[2][8], int *mg, int *eg);
void evaluate_rook_placement(int sq, int is_w, const int pawns[2][8], int *mg, int *eg);
void evaluate_king_safety(const Pos *p, int sq, int is_w, int *mg);

#endif // EVALUATE_H