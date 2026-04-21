#include "evaluate.h"
#include "engine.h"
#include "types.h"
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
 * @brief Evaluation constants for pawn structure.
 */
static const int doubled_pawn_penalty_mg = -15;
static const int doubled_pawn_penalty_eg = -20;
static const int isolated_pawn_penalty = -20;
static const int passed_pawn_bonus_mg = 10;
static const int passed_pawn_bonus_eg = 20;

/**
 * @brief Evaluation constants for rook placement.
 */
static const int rook_open_file_bonus = 25;
static const int rook_semi_open_file_bonus = 15;
static const int rook_on_seventh_bonus = 30;

/**
 * @brief Evaluation constants for king safety.
 */
static const int king_shield_missing_side_penalty = -15;
static const int king_shield_missing_center_penalty = -20;

/**
 * @brief Evaluation constants for piece combinations.
 */
static const int bishop_pair_bonus_mg = 40;
static const int bishop_pair_bonus_eg = 50;

/**
 * @brief Middlegame Piece-Square Tables (PSTs) from the Pesto evaluation function.
 */
static const int mg_pawn_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
      0,   0,   0,   0,   0,   0,   0,   0, // Rank 8
     98, 134,  61,  95,  68, 126,  34, -11, // Rank 7
     -6,   7,  26,  31,  65,  56,  25, -20, // Rank 6
    -14,  13,   6,  21,  23,  12,  17, -23, // Rank 5
    -27,  -2,  -5,  12,  17,   6,  10, -25, // Rank 4
    -26,  -4,  -4, -10,   3,   3,  33, -12, // Rank 3
    -35,  -1, -20, -23, -15,  24,  38, -22, // Rank 2
      0,   0,   0,   0,   0,   0,   0,   0, // Rank 1
};

static const int mg_knight_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -167, -89, -34, -49,  61, -97, -15, -107, // Rank 8
     -73, -41,  72,  36,  23,  62,   7,  -17, // Rank 7
     -47,  60,  37,  65,  84, 129,  73,   44, // Rank 6
      -9,  17,  19,  53,  37,  69,  18,   22, // Rank 5
     -13,   4,  16,  13,  28,  19,  21,   -8, // Rank 4
     -23,  -9,  12,  10,  19,  17,  25,  -16, // Rank 3
     -29, -53, -12,  -3,  -1,  18, -14,  -19, // Rank 2
    -105, -21, -58, -33, -17, -28, -19,  -23, // Rank 1
};

static const int mg_bishop_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -29,   4, -82, -37, -25, -42,   7,  -8, // Rank 8
    -26,  16, -18, -13,  30,  59,  18, -47, // Rank 7
    -16,  37,  43,  40,  35,  50,  37,  -2, // Rank 6
     -4,   5,  19,  50,  37,  37,   7,  -2, // Rank 5
     -6,  13,  13,  26,  34,  12,  10,   4, // Rank 4
      0,  15,  15,  15,  14,  27,  18,  10, // Rank 3
      4,  15,  16,   0,   7,  21,  33,   1, // Rank 2
    -33,  -3, -14, -21, -13, -12, -39, -21, // Rank 1
};

static const int mg_rook_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
     32,  42,  32,  51, 63,  9,  31,  43, // Rank 8
     27,  32,  58,  62, 80, 67,  26,  44, // Rank 7
     -5,  19,  26,  36, 17, 45,  61,  16, // Rank 6
    -24, -11,   7,  26, 24, 35,  -8, -20, // Rank 5
    -36, -26, -12,  -1,  9, -7,   6, -23, // Rank 4
    -45, -25, -16, -17,  3,  0,  -5, -33, // Rank 3
    -44, -16, -20,  -9, -1, 11,  -6, -71, // Rank 2
    -19, -13,   1,  17, 16,  7, -37, -26, // Rank 1
};

static const int mg_queen_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -28,   0,  29,  12,  59,  44,  43,  45, // Rank 8
    -24, -39,  -5,   1, -16,  57,  28,  54, // Rank 7
    -13, -17,   7,   8,  29,  56,  47,  57, // Rank 6
    -27, -27, -16, -16,  -1,  17,  -2,   1, // Rank 5
     -9, -26,  -9, -10,  -2,  -4,   3,  -3, // Rank 4
    -14,   2, -11,  -2,  -5,   2,  14,   5, // Rank 3
    -35,  -8,  11,   2,   8,  15,  -3,   1, // Rank 2
     -1, -18,  -9,  10, -15, -25, -31, -50, // Rank 1
};

static const int mg_king_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -65,  23,  16, -15, -56, -34,   2,  13, // Rank 8
     29,  -1, -20,  -7,  -8,  -4, -38, -29, // Rank 7
     -9,  24,   2, -16, -20,   6,  22, -22, // Rank 6
    -17, -20, -12, -27, -30, -25, -14, -36, // Rank 5
    -49,  -1, -27, -39, -46, -44, -33, -51, // Rank 4
    -14, -14, -22, -46, -44, -30, -15, -27, // Rank 3
      1,   7,  -8, -64, -43, -16,   9,   8, // Rank 2
    -15,  36,  12, -54,   8, -28,  24,  14, // Rank 1
};

/**
 * @brief Endgame Piece-Square Tables (PSTs) from the Pesto evaluation function.
 */
static const int eg_pawn_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
      0,   0,   0,   0,   0,   0,   0,   0, // Rank 8
    178, 173, 158, 134, 147, 132, 165, 187, // Rank 7
    104, 100,  95,  85,  69,  86,  84,  90, // Rank 6
     51,  60,  55,  55,  51,  58,  60,  48, // Rank 5
     24,  27,  29,  25,  20,  23,  22,  18, // Rank 4
     13,  14,  13,  10,   9,  10,  13,   3, // Rank 3
      8,   8,   7,   6,   5,   6,   7,   8, // Rank 2
      0,   0,   0,   0,   0,   0,   0,   0, // Rank 1
};

static const int eg_knight_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -58, -38, -13, -28, -31, -27, -63, -99, // Rank 8
    -25,  -8, -25,  -2,  -9, -25, -24, -52, // Rank 7
    -24, -20,  10,   9,  -1,  -9, -19, -41, // Rank 6
    -17,   3,  22,  22,  22,  11,   8, -18, // Rank 5
    -18,  -6,  16,  25,  16,  17,   4, -18, // Rank 4
    -23,  -3,  -1,  15,  10,  -3, -20, -22, // Rank 3
    -42, -20, -10,  -5,  -2, -20, -23, -44, // Rank 2
    -29, -51, -23, -15, -22, -18, -50, -64, // Rank 1
};

static const int eg_bishop_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -14, -21, -11,  -8, -7,  -9, -17, -24, // Rank 8
     -8,  -4,   7, -12, -3, -13,  -4, -14, // Rank 7
      2,  -8,   0,  -1, -2,   6,   0,   4, // Rank 6
     -3,   9,  12,   9,  7,  10,   3,   2, // Rank 5
     -6,   3,  13,  19,  7,  10,  -3,  -9, // Rank 4
    -12,  -3,   8,  10, 13,   3,  -7, -15, // Rank 3
    -14, -18,  -7,  -1,  4,  -9, -15, -27, // Rank 2
    -23,  -9, -23,  -5, -9, -16,  -5, -17, // Rank 1
};

static const int eg_rook_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    13, 10, 18, 15, 12,  12,   8,   5, // Rank 8
    11, 13, 13, 11, -3,   3,   8,   3, // Rank 7
     7,  7,  7,  5,  4,  -3,  -5,  -3, // Rank 6
     4,  3, 13,  1,  2,   1,  -1,   2, // Rank 5
     3,  5,  8,  4, -5,  -6,  -8, -11, // Rank 4
    -4,  0, -5, -1, -7, -12,  -8, -16, // Rank 3
    -6, -6,  0,  2, -9,  -9, -11,  -3, // Rank 2
    -9,  2,  3, -1, -5, -13,   4, -20, // Rank 1
};

static const int eg_queen_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
     -9,  22,  22,  27,  27,  19,  10,  20, // Rank 8
    -17,  20,  32,  41,  58,  25,  30,   0, // Rank 7
    -20,   6,   9,  49,  47,  35,  19,   9, // Rank 6
      3,  22,  24,  45,  57,  40,  57,  36, // Rank 5
    -18,  28,  19,  47,  31,  34,  39,  23, // Rank 4
    -16, -27,  15,   6,   9,  17,  10,   5, // Rank 3
    -22, -23, -30, -16, -16, -23, -36, -32, // Rank 2
    -33, -28, -22, -43,  -5, -32, -20, -41, // Rank 1
};

static const int eg_king_pst[64] = { // From White's perspective, array indices map from Rank 8 down to Rank 1
    -74, -35, -18, -18, -11,  15,   4, -17, // Rank 8
    -12,  17,  14,  17,  17,  38,  23,  11, // Rank 7
     10,  17,  23,  15,  20,  45,  44,  13, // Rank 6
     -8,  22,  24,  27,  26,  33,  26,   3, // Rank 5
    -18,  -4,  21,  24,  27,  23,   9, -11, // Rank 4
    -19,  -3,  11,  21,  23,  16,   7,  -9, // Rank 3
    -27, -11,   4,  13,  15,   4,  -5, -17, // Rank 2
    -53, -34, -21, -11, -28, -14, -24, -43, // Rank 1
};

/**
 * @brief Gets the Piece-Square Table index for a given square and color.
 * The PSTs are defined from White's perspective (rank 1 to 8), so for Black,
 * the rank needs to be flipped.
 * @param sq The square index (0-63).
 * @param is_w 1 if the piece is white, 0 if black.
 * @return The corresponding index (0-63) into the PST array.
 */
static int get_pst_sq(int sq, int is_w) {
    int r = sq / 8;
    int f = sq % 8;
    int pst_r = is_w ? (7 - r) : r;
    return pst_r * 8 + f;
}

/**
 * @brief Evaluates pawn-specific features like doubled, isolated, and passed pawns.
 * @param sq The pawn's square.
 * @param is_w The pawn's color.
 * @param pawns Pre-calculated pawn counts per file for each color.
 * @param min_pawn_rank Pre-calculated minimum pawn rank per file for each color.
 * @param max_pawn_rank Pre-calculated maximum pawn rank per file for each color.
 * @param mg Pointer to the middlegame score to update.
 * @param eg Pointer to the endgame score to update.
 */
static void evaluate_pawn_structure(int sq, int is_w, const int pawns[2][8], const int min_pawn_rank[2][8], const int max_pawn_rank[2][8], int *mg, int *eg) {
    int r = sq / 8;
    int f = sq % 8;
    int enemy = !is_w;

    // Evaluate doubled pawns
    if (pawns[is_w][f] > 1) {
        *mg += doubled_pawn_penalty_mg;
        *eg += doubled_pawn_penalty_eg;
    }

    // Evaluate isolated pawns
    int isolated = 1;
    if (f > 0 && pawns[is_w][f - 1] > 0) isolated = 0;
    if (f < 7 && pawns[is_w][f + 1] > 0) isolated = 0;
    if (isolated) {
        *mg += isolated_pawn_penalty;
        *eg += isolated_pawn_penalty;
    }

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
        *mg += r_bonus * passed_pawn_bonus_mg;
        *eg += r_bonus * passed_pawn_bonus_eg;
    }
}

/**
 * @brief Evaluates rook-specific features like open files and 7th rank presence.
 * @param sq The rook's square.
 * @param is_w The rook's color.
 * @param pawns Pre-calculated pawn counts per file for each color.
 * @param mg Pointer to the middlegame score to update.
 * @param eg Pointer to the endgame score to update.
 */
static void evaluate_rook_placement(int sq, int is_w, const int pawns[2][8], int *mg, int *eg) {
    int r = sq / 8;
    int f = sq % 8;
    int enemy = !is_w;

    // Bonus for rooks on open or semi-open files.
    if (pawns[is_w][f] == 0) { // No friendly pawns on this file.
        if (pawns[enemy][f] == 0) { // No enemy pawns either: fully open file.
            *mg += rook_open_file_bonus;
            *eg += rook_open_file_bonus;
        } else { // Enemy pawns exist: semi-open file.
            *mg += rook_semi_open_file_bonus;
            *eg += rook_semi_open_file_bonus;
        }
    }
    // Evaluate 7th rank positioning
    if ((is_w && r == 6) || (!is_w && r == 1)) {
        *mg += rook_on_seventh_bonus;
        *eg += rook_on_seventh_bonus;
    }
}

/**
 * @brief Evaluates king safety by checking for a protective pawn shield.
 * @param p The board position.
 * @param sq The king's square.
 * @param is_w The king's color.
 * @param mg Pointer to the middlegame score to update (safety is mostly a middlegame factor).
 */
static void evaluate_king_safety(const Pos *p, int sq, int is_w, int *mg) {
    int r = sq / 8;
    int f = sq % 8;
    
    int shield_rank = is_w ? (r + 1) : (r - 1);
    // Only evaluate pawn shield if it's on the board
    if (shield_rank < 0 || shield_rank > 7) return;

    char friendly_pawn = is_w ? 'P' : 'p';

    if (f == 0 || p->b[shield_rank * 8 + f - 1] != friendly_pawn) *mg += king_shield_missing_side_penalty;
    if (p->b[shield_rank * 8 + f] != friendly_pawn) *mg += king_shield_missing_center_penalty;
    if (f == 7 || p->b[shield_rank * 8 + f + 1] != friendly_pawn) *mg += king_shield_missing_side_penalty;
}

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
    
    // --- Phase 1: Pre-evaluation analysis ---
    // Gather pawn structure data and count non-pawn material for game phase calculation.
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
    if (game_phase > 24) game_phase = 24; // Cap game phase at 24

    // --- Phase 2: Piece-by-piece evaluation ---
    for (int sq = 0; sq < 64; sq++) {
        char pc = p->b[sq];
        if (pc == '.') continue;
        
        int is_w = is_white_piece(pc);
        char up = (char)toupper((unsigned char)pc);
        
        int pst_sq = get_pst_sq(sq, is_w);
        
        int mg = 0, eg = 0;
        
        switch (up) {
            case 'P':
                mg = mg_pawn_value + mg_pawn_pst[pst_sq];
                eg = eg_pawn_value + eg_pawn_pst[pst_sq];
                evaluate_pawn_structure(sq, is_w, pawns, min_pawn_rank, max_pawn_rank, &mg, &eg);
                break;
            case 'N':
                mg = mg_knight_value + mg_knight_pst[pst_sq];
                eg = eg_knight_value + eg_knight_pst[pst_sq];
                break;
            case 'B':
                mg = mg_bishop_value + mg_bishop_pst[pst_sq];
                eg = eg_bishop_value + eg_bishop_pst[pst_sq];
                break;
            case 'R':
                mg = mg_rook_value + mg_rook_pst[pst_sq];
                eg = eg_rook_value + eg_rook_pst[pst_sq];
                evaluate_rook_placement(sq, is_w, pawns, &mg, &eg);
                break;
            case 'Q':
                mg = mg_queen_value + mg_queen_pst[pst_sq];
                eg = eg_queen_value + eg_queen_pst[pst_sq];
                break;
            case 'K':
                mg = 20000 + mg_king_pst[pst_sq];
                eg = 20000 + eg_king_pst[pst_sq];
                evaluate_king_safety(p, sq, is_w, &mg);
                break;
        }
        
        mg_score[is_w] += mg;
        eg_score[is_w] += eg;
    }
    
    // --- Phase 3: Global bonuses and final calculation ---
    // Apply bishop pair bonus
    if (bishops[1] >= 2) { mg_score[1] += bishop_pair_bonus_mg; eg_score[1] += bishop_pair_bonus_eg; }
    if (bishops[0] >= 2) { mg_score[0] += bishop_pair_bonus_mg; eg_score[0] += bishop_pair_bonus_eg; }
    
    // Calculate tapered score
    int mg_eval = mg_score[1] - mg_score[0];
    int eg_eval = eg_score[1] - eg_score[0];
    
    int mg_weight = game_phase;
    int eg_weight = 24 - game_phase;
    
    int score = (mg_eval * mg_weight + eg_eval * eg_weight) / 24;
    
    // Return score from the perspective of the side to move
    return p->white_to_move ? score : -score;
}