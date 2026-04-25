#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> // For pipe, dup2, close, read
#include "../src/engine.h"
#include "../src/algorithm.h"
#include "../src/types.h"
#include "../src/evaluate.h"

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

// Helper functions to capture stdout
static int stdout_pipe[2];
static int original_stdout_fd;
static char stdout_buffer[256];

/**
 * @brief Redirects stdout to an internal buffer to capture output.
 */
static void redirect_stdout_to_buffer() {
    fflush(stdout);
    original_stdout_fd = dup(STDOUT_FILENO);
    pipe(stdout_pipe);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);
}

/**
 * @brief Restores stdout and returns the captured output as a string.
 */
static char* restore_stdout_and_get_output() {
    fflush(stdout);
    dup2(original_stdout_fd, STDOUT_FILENO);
    close(original_stdout_fd);
    
    memset(stdout_buffer, 0, sizeof(stdout_buffer));
    read(stdout_pipe[0], stdout_buffer, sizeof(stdout_buffer) - 1);
    close(stdout_pipe[0]);
    return stdout_buffer;
}

/**
 * @brief Helper to manually set up a Pos struct to the standard starting position.
 */
static void setup_start_pos(Pos *p) {
    const char startpos_visual[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p->b[(7 - r) * 8], startpos_visual[r], 8);
    }
    p->white_to_move = 1;
    p->castling = 1 | 2 | 4 | 8;
    p->ep = -1;
}

/**
 * @brief Tests that negamax can find a forced mate in one move.
 */
static void test_negamax_mate_in_one(void) {
    Pos p;
    Move best_move;
    // Position: Fool's Mate. Black to move and deliver checkmate with Qh4 (d8 -> h4).
    // FEN: rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2
    const char board_visual[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', '.', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', 'p', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', 'P', '.'},
        {'.', '.', '.', '.', '.', 'P', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', '.', '.', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    };
    for (int r = 0; r < 8; r++) memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    p.white_to_move = 0;
    p.castling = 1 | 2 | 4 | 8;
    p.ep = -1;
    
    // Set up the search environment.
    stop_search = 0;
    nodes = 0;
    stop_time = get_time_ms() + 1000; // Allow 1 second for the search.

    // Search with a depth of 2, which is enough to see the mate.
    int score = negamax(&p, 2, -30000, 30000, &best_move);
    
    printf("\n\t[DEBUG] Testing negamax for mate-in-1.\n");
    
    // The score should be the checkmate value.
    CU_ASSERT(score > 19000);
    printf("\n\t\t[DEBUG] Score for mate-in-1: %d\n", score);
    printf("\t\t[DEBUG] Expected: > 19000\n");
    
    // The best move should be Qh4.
    // Queen from d8 (59) to h4 (31).
    CU_ASSERT_EQUAL(best_move.from, 59);
    printf("\n\t\t[DEBUG] Best move 'from' square: %d\n", best_move.from);
    printf("\t\t[DEBUG] Expected: 59 (d8)\n");
    
    CU_ASSERT_EQUAL(best_move.to, 31);
    printf("\n\t\t[DEBUG] Best move 'to' square: %d\n", best_move.to);
    printf("\t\t[DEBUG] Expected: 31 (h4)\n");
}

/**
 * @brief Tests that negamax correctly identifies a stalemate position.
 */
static void test_negamax_stalemate(void) {
    Pos p;
    Move best_move;
    // Position: Black is in stalemate. King on h8 is not in check but has no legal moves.
    // FEN: 7k/5Q2/8/8/8/8/8/K7 b - - 0 1
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', '.', '.', '.', 'k'},
        {'.', '.', '.', '.', '.', 'Q', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'K', '.', '.', '.', '.', '.', '.', '.'}
    };
    for (int r = 0; r < 8; r++) memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    p.white_to_move = 0;
    p.castling = 0;
    p.ep = -1;
    
    // Set up the search environment.
    stop_search = 0;
    nodes = 0;
    stop_time = get_time_ms() + 1000; // Allow 1 second for the search.

    // Search with any depth.
    int score = negamax(&p, 2, -30000, 30000, &best_move);
    
    printf("\n\t[DEBUG] Testing negamax for stalemate detection.\n");
    
    // The score for a stalemate is 0.
    CU_ASSERT_EQUAL(score, 0);
    printf("\n\t\t[DEBUG] Score for stalemate: %d\n", score);
    printf("\t\t[DEBUG] Expected: 0\n");
}

/**
 * @brief Tests that negamax handles an invalid (negative) depth gracefully.
 */
static void test_negamax_invalid_depth(void) {
    Pos p;
    Move best_move;
    // Use the starting position for simplicity.
    setup_start_pos(&p);

    // Set up the search environment.
    stop_search = 0;
    nodes = 0;
    stop_time = get_time_ms() + 1000;

    printf("\n\t[DEBUG] Testing negamax with invalid depth = -1.\n");

    // With a negative depth, negamax should immediately return the static evaluation.
    int expected_score = evaluate(&p);
    int score = negamax(&p, -1, -30000, 30000, &best_move);

    CU_ASSERT_EQUAL(score, expected_score);
    printf("\n\t\t[DEBUG] Score for invalid depth: %d\n", score);
    printf("\t\t[DEBUG] Expected score (from evaluate()): %d\n", expected_score);
}

/**
 * @brief Tests that search_position finds a mate-in-1 and prints the correct move.
 */
static void test_search_finds_mate_in_one(void) {
    Pos p;
    // FEN: rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2
    const char board_visual[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', '.', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', 'p', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', 'P', '.'},
        {'.', '.', '.', '.', '.', 'P', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', '.', '.', 'P'},
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
    };
    for (int r = 0; r < 8; r++) memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    p.white_to_move = 0;
    p.castling = 1 | 2 | 4 | 8;
    p.ep = -1;

    redirect_stdout_to_buffer();
    search_position(&p, "go movetime 500");
    char* output = restore_stdout_and_get_output();

    // Trim trailing newline for comparison
    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position for mate-in-1.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    printf("\t\t[DEBUG] Expected: 'bestmove d8h4'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove d8h4");
}

/**
 * @brief Tests that search_position correctly handles a stalemate.
 */
static void test_search_handles_stalemate(void) {
    Pos p;
    // FEN: 7k/5Q2/8/8/8/8/8/K7 b - - 0 1
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', '.', '.', '.', 'k'},
        {'.', '.', '.', '.', '.', 'Q', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'K', '.', '.', '.', '.', '.', '.', '.'}
    };
    for (int r = 0; r < 8; r++) memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    p.white_to_move = 0;
    p.castling = 0;
    p.ep = -1;

    redirect_stdout_to_buffer();
    search_position(&p, "go movetime 500");
    char* output = restore_stdout_and_get_output();

    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position for stalemate.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    printf("\t\t[DEBUG] Expected: 'bestmove 0000'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove 0000");
}

/**
 * @brief Tests that search_position uses its fallback move on an immediate timeout.
 */
static void test_search_fallback_on_immediate_timeout(void) {
    Pos p;
    setup_start_pos(&p);

    redirect_stdout_to_buffer();
    search_position(&p, "go movetime 1");
    char* output = restore_stdout_and_get_output();

    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position with immediate timeout.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    printf("\t\t[DEBUG] Expected: 'bestmove b1a3'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove b1a3");
}

/**
 * @brief Tests that search_position correctly handles a checkmate position.
 */
static void test_search_handles_checkmate(void) {
    Pos p;
    // FEN: 2k5/8/8/8/8/8/5PPP/2r4K w - - 0 1
    // White is in checkmate.
    const char board_visual[8][8] = {
        {'.', '.', 'k', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', 'P', 'P', 'P'},
        {'.', '.', 'r', '.', '.', '.', '.', 'K'}
    };
    for (int r = 0; r < 8; r++) memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;

    redirect_stdout_to_buffer();
    search_position(&p, "go movetime 500");
    char* output = restore_stdout_and_get_output();

    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position for checkmate.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    printf("\t\t[DEBUG] Expected: 'bestmove 0000'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove 0000");
}


/**
 * @brief Tests robustness against a non-numeric movetime value.
 */
static void test_search_movetime_non_numeric(void) {
    Pos p;
    setup_start_pos(&p);

    // "go movetime abc" should be handled gracefully. atoll("abc") returns 0,
    // which should lead to an immediate timeout and a fallback move.
    redirect_stdout_to_buffer();
    search_position(&p, "go movetime abc");
    char* output = restore_stdout_and_get_output();

    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position with non-numeric movetime.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    printf("\t\t[DEBUG] Expected: 'bestmove b1a3'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove b1a3");
}

/**
 * @brief Tests that search_position returns a valid move with a very short movetime.
 */
static void test_search_short_movetime(void) {
    Pos p;
    setup_start_pos(&p);

    // With a very short movetime (30ms), the engine should complete at least depth 1
    // and return a valid move before timing out, rather than relying on the final fallback.
    redirect_stdout_to_buffer();
    search_position(&p, "go movetime 30");
    char* output = restore_stdout_and_get_output();

    size_t len = strlen(output);
    if (len > 0 && output[len - 1] == '\n') output[len - 1] = '\0';

    printf("\n\t[DEBUG] Testing search_position with short movetime.\n");
    printf("\n\t\t[DEBUG] Output: '%s'\n", output);
    // With the short movetime, the engine doesn't generate a lot of nodes
    // The move generation starts from a1, which is a rook, but it can't move.
    // The knight on b1 is the first piece that can move. Its move to c3 has a better positional score.
    printf("\t\t[DEBUG] Expected: 'bestmove b1c3'\n");
    CU_ASSERT_STRING_EQUAL(output, "bestmove b1c3");
}

int main(void) {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("AlgorithmTestSuite", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite, "test_negamax_mate_in_one", test_negamax_mate_in_one)) ||
        (NULL == CU_add_test(pSuite, "test_negamax_stalemate", test_negamax_stalemate)) ||
        (NULL == CU_add_test(pSuite, "test_negamax_invalid_depth", test_negamax_invalid_depth)) ||
        (NULL == CU_add_test(pSuite, "test_search_finds_mate_in_one", test_search_finds_mate_in_one)) ||
        (NULL == CU_add_test(pSuite, "test_search_handles_stalemate", test_search_handles_stalemate)) ||
        (NULL == CU_add_test(pSuite, "test_search_fallback_on_immediate_timeout", test_search_fallback_on_immediate_timeout)) ||
        (NULL == CU_add_test(pSuite, "test_search_handles_checkmate", test_search_handles_checkmate)) ||
        (NULL == CU_add_test(pSuite, "test_search_movetime_non_numeric", test_search_movetime_non_numeric)) ||
        (NULL == CU_add_test(pSuite, "test_search_short_movetime", test_search_short_movetime))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    unsigned int failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return (failures > 0) ? 1 : 0;
}