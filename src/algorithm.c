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