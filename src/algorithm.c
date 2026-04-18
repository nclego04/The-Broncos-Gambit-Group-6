#include "algorithm.h"
#include "evaluate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "engine.h"
#include "types.h"
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <time.h>
#endif

static long long stop_time = 0;
static int stop_search = 0;
static int nodes = 0;

/**
 * @brief Gets the current system time in milliseconds.
 * Handles cross-platform differences between Windows and POSIX systems.
 */
long long get_time_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;
#endif
}

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
int negamax(const Pos *p, int depth, int alpha, int beta, Move *best_move) {
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
