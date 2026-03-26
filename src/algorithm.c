#include "algorithm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

static long long stop_time = 0;
static int stop_search = 0;
static int nodes = 0;

static long long get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// Basic Piece-Square Tables (simplified, mapping center control and piece activity)
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

static int evaluate(const Pos *p) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int sq = r * 8 + f;
            char pc = p->b[sq];
            if (pc == '.') continue;
            
            int is_white = is_white_piece(pc);
            char up = (char)toupper((unsigned char)pc);
            
            int pst_r = is_white ? (7 - r) : r;
            int pst_sq = pst_r * 8 + f;
            
            int val = 0;
            switch (up) {
                case 'P': val = 100 + pawn_pst[pst_sq]; break;
                case 'N': val = 320 + knight_pst[pst_sq]; break;
                case 'B': val = 330 + bishop_pst[pst_sq]; break;
                case 'R': val = 500 + rook_pst[pst_sq]; break;
                case 'Q': val = 900 + queen_pst[pst_sq]; break;
                case 'K': val = 20000 + king_pst[pst_sq]; break;
            }
            score += is_white ? val : -val;
        }
    }
    return p->white_to_move ? score : -score;
}

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
    
    Move best_move = {0, 0, 0};
    Move current_best = {0, 0, 0};
    int completed_depth = 0;
    
    // Iterative Deepening
    for (int depth = 1; depth <= 64; depth++) {
        int score = negamax(p, depth, -30000, 30000, &current_best);
        if (stop_search) break;
        best_move = current_best;
        completed_depth = depth;
        if (score > 19000 || score < -19000) break; // Stop early on mate
        if (get_time_ms() - start_time > movetime / 2) break; // Risking overstep on next depth
    }
    
    // Fallback if no move assigned or stopped entirely at depth 1
    if (best_move.from == best_move.to) {
        Move ms[256];
        if (legal_moves(p, ms) > 0) best_move = ms[0];
    }
    
    long long elapsed = get_time_ms() - start_time;
    if (elapsed == 0) elapsed = 1; // Prevent division by zero
    long long nps = (long long)nodes * 1000 / elapsed;

    FILE *f = fopen("tests/search_metrics.txt", "a");
    if (f) {
        fprintf(f, "Depth: %d | Nodes: %d | Time: %lld ms | NPS: %lld\n", completed_depth, nodes, elapsed, nps);
        fclose(f);
    }

    if (best_move.from == 0 && best_move.to == 0) {
        printf("bestmove 0000\n");
        fflush(stdout);
    } else {
        print_bestmove(best_move);
    }
}
