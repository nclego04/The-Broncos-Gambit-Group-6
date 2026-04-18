#include "evaluate.h"
#include "engine.h"
#include <ctype.h>

/**
 * @brief Material values for each piece in middlegame and endgame, in centipawns.
 * These can be tuned to adjust the engine's play style.
 */
static const int mg_pawn_value = 100;
static const int eg_pawn_value = 120;
static const int mg_knight_value = 320;
static const int eg_knight_value = 300;
static const int mg_bishop_value = 330;
static const int eg_bishop_value = 330;
static const int mg_rook_value = 500;
static const int eg_rook_value = 500;
static const int mg_queen_value = 900;
static const int eg_queen_value = 900;

/**
 * @brief Middlegame Piece-Square Tables (PSTs) from the Pesto evaluation function.
 */
static const int mg_pawn_pst[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0,
};

static const int mg_knight_pst[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

static const int mg_bishop_pst[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

static const int mg_rook_pst[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

static const int mg_queen_pst[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

static const int mg_king_pst[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

/**
 * @brief Endgame Piece-Square Tables (PSTs) from the Pesto evaluation function.
 */
static const int eg_pawn_pst[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
    104, 100,  95,  85,  69,  86,  84,  90,
     51,  60,  55,  55,  51,  58,  60,  48,
     24,  27,  29,  25,  20,  23,  22,  18,
     13,  14,  13,  10,   9,  10,  13,   3,
      8,   8,   7,   6,   5,   6,   7,   8,
      0,   0,   0,   0,   0,   0,   0,   0,
};

static const int eg_knight_pst[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

static const int eg_bishop_pst[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9,  7,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

static const int eg_rook_pst[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

static const int eg_queen_pst[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

static const int eg_king_pst[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  15,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

/**
 * @brief Statically evaluates the board state from the perspective of the side to move.
 * Factors in material, Piece-Square Tables (PSTs), game phase (tapered evaluation),
 * pawn structures, and king safety.
 * @param p The board position to evaluate.
 * @return The evaluation score in centipawns.
 */
int evaluate(const Pos *p) {
    int mg_score[2] = {0, 0}; // Score arrays: [0] Black, [1] White
    int eg_score[2] = {0, 0};
    int game_phase = 0;
    
    int pawns[2][8] = {{0}}; 
    int min_pawn_rank[2][8];
    int max_pawn_rank[2][8];
    for (int c = 0; c < 2; c++) {
        for (int f = 0; f < 8; f++) {
            min_pawn_rank[c][f] = 8;
            max_pawn_rank[c][f] = -1;
        }
    }
    int bishops[2] = {0, 0};

    // Phase 1: Gather structural data and calculate game phase
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char pc = p->b[r * 8 + f];
            if (pc == '.') continue;
            
            int is_w = is_white_piece(pc);
            char up = (char)toupper((unsigned char)pc);
            
            if (up == 'P') {
                pawns[is_w][f]++;
                if (r < min_pawn_rank[is_w][f]) min_pawn_rank[is_w][f] = r;
                if (r > max_pawn_rank[is_w][f]) max_pawn_rank[is_w][f] = r;
            } else if (up == 'B') {
                bishops[is_w]++;
                game_phase += 1;
            } else if (up == 'N') {
                game_phase += 1;
            } else if (up == 'R') {
                game_phase += 2;
            } else if (up == 'Q') {
                game_phase += 4;
            }
        }
    }
    if (game_phase > 24) game_phase = 24;

    // Phase 2: Evaluate pieces
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            char pc = p->b[sq];
            if (pc == '.') continue;
            
            int is_w = is_white_piece(pc);
            char up = (char)toupper((unsigned char)pc);
            
            int pst_r = is_w ? (7 - r) : r;
            int pst_sq = pst_r * 8 + f;
            
            int mg = 0, eg = 0;
            int enemy = !is_w;
            
            switch (up) {
                case 'P': {
                    mg += mg_pawn_value + mg_pawn_pst[pst_sq];
                    eg += eg_pawn_value + eg_pawn_pst[pst_sq];
                    
                    // Evaluate doubled pawns
                    if (pawns[is_w][f] > 1) { mg -= 15; eg -= 20; }
                    
                    // Evaluate isolated pawns
                    int isolated = 1;
                    if (f > 0 && pawns[is_w][f - 1] > 0) isolated = 0;
                    if (f < 7 && pawns[is_w][f + 1] > 0) isolated = 0;
                    if (isolated) { mg -= 20; eg -= 20; }
                    
                    // Evaluate passed pawns
                    int passed = 1;
                    if (is_w) {
                        if (max_pawn_rank[enemy][f] > r) passed = 0;
                        if (f > 0 && max_pawn_rank[enemy][f - 1] > r) passed = 0;
                        if (f < 7 && max_pawn_rank[enemy][f + 1] > r) passed = 0;
                    } else {
                        if (min_pawn_rank[enemy][f] != 8 && min_pawn_rank[enemy][f] < r) passed = 0;
                        if (f > 0 && min_pawn_rank[enemy][f - 1] != 8 && min_pawn_rank[enemy][f - 1] < r) passed = 0;
                        if (f < 7 && min_pawn_rank[enemy][f + 1] != 8 && min_pawn_rank[enemy][f + 1] < r) passed = 0;
                    }
                    if (passed) {
                        int r_bonus = is_w ? r : (7 - r);
                        mg += r_bonus * 10;
                        eg += r_bonus * 20;
                    }
                    break;
                }
                case 'N':
                    mg += mg_knight_value + mg_knight_pst[pst_sq];
                    eg += eg_knight_value + eg_knight_pst[pst_sq];
                    break;
                case 'B':
                    mg += mg_bishop_value + mg_bishop_pst[pst_sq];
                    eg += eg_bishop_value + eg_bishop_pst[pst_sq];
                    break;
                case 'R':
                    mg += mg_rook_value + mg_rook_pst[pst_sq];
                    eg += eg_rook_value + eg_rook_pst[pst_sq];

                    // Bonus for rooks on open or semi-open files.
                    if (pawns[is_w][f] == 0) { // No friendly pawns on this file.
                        if (pawns[enemy][f] == 0) { // No enemy pawns either: fully open file.
                            mg += 25;
                            eg += 25;
                        } else { // Enemy pawns exist: semi-open file.
                            mg += 15;
                            eg += 15;
                        }
                    }
                    // Evaluate 7th rank positioning
                    if ((is_w && r == 6) || (!is_w && r == 1)) {
                        mg += 30; eg += 30;
                    }
                    break;
                case 'Q':
                    mg += mg_queen_value + mg_queen_pst[pst_sq];
                    eg += eg_queen_value + eg_queen_pst[pst_sq];
                    break;
                case 'K':
                    mg += 20000 + mg_king_pst[pst_sq];
                    eg += 20000 + eg_king_pst[pst_sq];
                    
                    // King safety: check for friendly pawns on the 3 squares directly in front.
                    // If king is on the edge of the board, still apply penalty due to mobility reduction
                    int shield_rank = is_w ? (r + 1) : (r - 1);
                    char friendly_pawn = is_w ? 'P' : 'p';

                    // Penalty for missing shield pawn to the left.
                    if (f == 0 || p->b[shield_rank * 8 + f - 1] != friendly_pawn) {
                        mg -= 15;
                    }
                    // Penalty for missing shield pawn in the center.
                    if (p->b[shield_rank * 8 + f] != friendly_pawn) {
                        mg -= 20;
                    }
                    // Penalty for missing shield pawn to the right.
                    if (f == 7 || p->b[shield_rank * 8 + f + 1] != friendly_pawn) {
                        mg -= 15;
                    }
                    break;
            }
            
            mg_score[is_w] += mg;
            eg_score[is_w] += eg;
        }
    }
    
    // Apply bishop pair bonus
    if (bishops[1] >= 2) { mg_score[1] += 40; eg_score[1] += 50; }
    if (bishops[0] >= 2) { mg_score[0] += 40; eg_score[0] += 50; }
    
    int mg_eval = mg_score[1] - mg_score[0];
    int eg_eval = eg_score[1] - eg_score[0];
    
    int mg_weight = game_phase;
    int eg_weight = 24 - game_phase;
    
    int score = (mg_eval * mg_weight + eg_eval * eg_weight) / 24;
    
    return p->white_to_move ? score : -score;
}