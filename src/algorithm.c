#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#endif

static long long stop_time = 0;
static int stop_search = 0;
static int nodes = 0;

/**
 * @brief The core Negamax search algorithm with Alpha-Beta pruning.
 * Recursively explores the game tree up to the specified depth.
 * @param p The current board position.
 * @param depth The remaining search depth.
 * @param alpha The lower bound for the search window.
 * @param beta The upper bound for the search window.
 * @param best_move Pointer to store the best move found at the root.
 * @return The best score found in centipawns.
 */
static int negamax(const Pos *p, int depth, int alpha, int beta, Move *best_move) {
    if ((nodes++ & 2047) == 0 && get_time_ms() >= stop_time) {
        stop_search = 1;
        return 0;
    }
    
    if (depth == 0) return evaluate(p);
    
    Move moves[256];
    int num_moves = legal_moves(p, moves);
    
    if (num_moves == 0) {
        int ksq = -1;
        char king = p->white_to_move ? 'K' : 'k';
        for (int i = 0; i < 64; i++) if (p->b[i] == king) { ksq = i; break; }
        if (ksq >= 0 && is_square_attacked(p, ksq, !p->white_to_move)) {
            return -20000 + (64 - depth); // Checkmate
        }
        return 0; // Stalemate
    }
    
    int best_score = -30000;
    for (int i = 0; i < num_moves; i++) {
        Pos np = make_move(p, moves[i]);
        int score = -negamax(&np, depth - 1, -beta, -alpha, NULL);
        if (stop_search) return 0;
        
        if (score > best_score) {
            best_score = score;
            if (best_move) *best_move = moves[i];
        }
        if (best_score > alpha) alpha = best_score;
        if (alpha >= beta) break; // Alpha-Beta cutoff
    }
    return best_score;
}

/**
 * @brief Endgame Piece-Square Tables (PSTs) for evaluating piece positioning in the late game.
 */
static const int eg_pawn_pst[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    80, 80, 80, 80, 80, 80, 80, 80,
    50, 50, 50, 50, 50, 50, 50, 50,
    30, 30, 30, 30, 30, 30, 30, 30,
    20, 20, 20, 20, 20, 20, 20, 20,
    10, 10, 10, 10, 10, 10, 10, 10,
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int eg_king_pst[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

/**
 * @brief Gets the current system time in milliseconds.
 * Handles cross-platform differences between Windows and POSIX systems.
 */
static long long get_time_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    return GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
#endif
}

/**
 * @brief Initiates the engine's time-controlled iterative deepening search.
 * Handles parsing the UCI 'go' command, managing search time, and outputting the best move.
 * @param p The starting board position.
 * @param go_cmd The UCI go command string containing time limits.
 */
void search_position(const Pos *p, const char *go_cmd) {
    long long movetime = 1000; 
    const char *ptr = strstr(go_cmd, "movetime");
    if (ptr) movetime = atoll(ptr + 8);
    else {
        ptr = strstr(go_cmd, "wtime");
        if (ptr) {
            // Fallback estimation using roughly 1/30th of remaining time
            long long wtime = 1000, btime = 1000;
            sscanf(ptr + 5, "%lld", &wtime);
            const char *bptr = strstr(go_cmd, "btime");
            if (bptr) sscanf(bptr + 5, "%lld", &btime);
            movetime = (p->white_to_move ? wtime : btime) / 30;
            if (movetime < 100) movetime = 100;
        }
    }
    
    long long start_time = get_time_ms();
    stop_time = start_time + movetime - 10;
    if (stop_time < start_time) stop_time = start_time;
    stop_search = nodes = 0;
    
    FILE *metrics_file = fopen("tests/search_metrics.txt", "a");
    if (metrics_file) {
        fprintf(metrics_file, "--- Searching Move ---\n");
        fclose(metrics_file);
    }

    Move best_move = {0, 0, 0};
    Move current_best = {0, 0, 0};
    
    // Iterative Deepening
    for (int depth = 1; depth <= 64; depth++) {
        int score = negamax(p, depth, -30000, 30000, &current_best);
        if (stop_search) break;
        best_move = current_best;
        
        long long current_elapsed = get_time_ms() - start_time;
        long long calc_elapsed = current_elapsed == 0 ? 1 : current_elapsed;
        long long current_nps = (long long)nodes * 1000 / calc_elapsed;
        
        metrics_file = fopen("tests/search_metrics.txt", "a");
        if (metrics_file) {
            fprintf(metrics_file, "Depth %2d | Nodes: %8d | Time: %5lld ms | NPS: %8lld\n", depth, nodes, current_elapsed, current_nps);
            fclose(metrics_file);
        }

        if (score > 19000 || score < -19000) break; // Stop early on mate
        if (get_time_ms() - start_time > movetime / 2) break; // Risking overstep on next depth
    }
    
    // Fallback if no move assigned or stopped entirely at depth 1
    if (best_move.from == best_move.to) {
        Move ms[256];
        if (legal_moves(p, ms) > 0) best_move = ms[0];
    }
    
    if (best_move.from == 0 && best_move.to == 0) {
        printf("bestmove 0000\n");
        fflush(stdout);
    } else {
        print_bestmove(best_move);
    }
}
/**
 * @brief Statically evaluates the board state from the perspective of the side to move.
 * Factors in material, Piece-Square Tables (PSTs), game phase (tapered evaluation),
 * pawn structures, and king safety.
 * @param p The board position to evaluate.
 * @return The evaluation score in centipawns.
 */
static int evaluate(const Pos *p) {
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
                    mg += 100 + pawn_pst[pst_sq];
                    eg += 120 + eg_pawn_pst[pst_sq];
                    
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
                    mg += 320 + knight_pst[pst_sq];
                    eg += 300 + knight_pst[pst_sq];
                    break;
                case 'B':
                    mg += 330 + bishop_pst[pst_sq];
                    eg += 330 + bishop_pst[pst_sq];
                    break;
                case 'R':
                    mg += 500 + rook_pst[pst_sq];
                    eg += 500 + rook_pst[pst_sq];
                    // Evaluate open files
                    if (pawns[0][f] == 0 && pawns[1][f] == 0) {
                        mg += 30; eg += 30;
                    }
                    // Evaluate 7th rank positioning
                    if ((is_w && r == 6) || (!is_w && r == 1)) {
                        mg += 30; eg += 30;
                    }
                    break;
                case 'Q':
                    mg += 900 + queen_pst[pst_sq];
                    eg += 900 + queen_pst[pst_sq];
                    break;
                case 'K':
                    mg += 20000 + king_pst[pst_sq];
                    eg += 20000 + eg_king_pst[pst_sq];
                    
                    // Evaluate king safety (pawn shields and open files)
                    if (f > 0 && pawns[is_w][f - 1] == 0) mg -= 15;
                    if (pawns[is_w][f] == 0) mg -= 20;
                    if (f < 7 && pawns[is_w][f + 1] == 0) mg -= 15;
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

/**
 * @brief Piece-Square Tables (PSTs) for evaluating piece positioning.
 */
static const int pawn_pst[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int knight_pst[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const int bishop_pst[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int rook_pst[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
      5, 10, 10, 10, 10, 10, 10,  5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      0,  0,  0,  5,  5,  0,  0,  0
};

static const int queen_pst[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int king_pst[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};