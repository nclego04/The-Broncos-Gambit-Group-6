#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We need to include the headers for the functions we want to test.
#include "../src/engine.h"
#include "../src/algorithm.h"
#include "../src/evaluate.h"
#include "../src/types.h"

/*
 * Test Suite setup and teardown functions.
 * These can be used to initialize and clean up resources for the tests.
 * For now, they are empty.
 */
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

static void test_evaluate_startpos(void) {
    Pos p;

    // Set up the board using a 2D array for visual clarity,
    // with White at the bottom (Rank 1).
    const char board_visual[8][8] = { // Board from White's perspective
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}, // Rank 8
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}, // Rank 2
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}  // Rank 1
    // {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}  // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}  // Rank 7
    // {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}  // Rank 8

    // Copy the visual board into the engine's 1D representation.
    // The engine sees Rank 1 (from board_visual[7]) at p.b[0-7], Rank 2 at p.b[8-15], etc.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 1 | 2 | 4 | 8; // All rights: WK, WQ, BK, BQ
    p.ep = -1;

    int score = evaluate(&p);
    printf("\n[DEBUG] Startpos score: %d\n", score);
    /* The starting position is equal (0). This is the same from Black's perspective. */
    CU_ASSERT_EQUAL(score, 0);
}

static void test_evaluate_material_advantage(void) {
    Pos p;

    // Position to test pure material advantage: White Knight vs Black lone King.
    // The knight is placed on e2, a square with minimal PST values, to better
    // isolate the material score from positional bonuses.
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', 'N', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', 'N', '.', '.', '.'}  // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 8

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    printf("\n[DEBUG] Material advantage score: %d\n", score); 
    printf("[DEBUG] Expected material advantage score: 298\n");
    /* Score reflects the knight's value, with minimal adjustment from PSTs.
     * game_phase=1 (one knight)
     * White N on e2 (sq 12 -> pst_sq 52): mg=-1, eg=-2
     * mg_eval = material value + pst bonus = 320 - 1 = 319
     * eg_eval = material value + pst bonus = 300 - 2 = 298
     * score = (mg_eval * 1 + eg_eval * 23) / 24 = (319 + 6854) / 24 = 7173 / 24 = 298.
     */
    CU_ASSERT_EQUAL(score, 298);
}

static void test_evaluate_positional_advantage(void) {
    Pos p;

    // Position to test pure positional advantage. Material is equal (K+N vs k+n).
    // White's knight is centralized on d4 (good), Black's is on the rim at h8 (bad).
    // Black's knight is placed on e7, a square with near-zero positional value,
    // to better isolate the advantage of White's well-placed knight.
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', 'n', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', 'N', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', 'N', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'.', '.', '.', '.', 'n', '.', '.', '.'}  // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 8

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    printf("\n[DEBUG] Positional advantage score: %d\n", score);
    printf("[DEBUG] Expected positional advantage score: 25\n");
    /* The score reflects the advantage of White's centralized knight vs Black's neutral one.
     * game_phase=2 (two knights).
     * White N on d4 (sq 27 -> pst_sq 35): mg=+13, eg=+25
     * Black n on e7 (sq 52 -> pst_sq 52): mg=-1, eg=-2
     * mg_eval = white_pst_bonus - black_pst_bonus = 13 - (-1) = 14
     * eg_eval = white_pst_bonus - black_pst_bonus = 25 - (-2) = 27
     * score = (mg_eval * 2 + eg_eval * 22) / 24 = (14*2 + 27*22) / 24 = 622 / 24 = 25.
     */
    CU_ASSERT_EQUAL(score, 25);
}

static void test_evaluate_pawn_structure(void) {
    Pos p;

    // Position to test the doubled pawn penalty, isolated pawn penalty, and passed pawns bonus.
    // The pawn structure is designed to isolate the penalty from other factors.
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', 'P', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', 'P', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', 'P', '.', '.', '.', '.'}  // Rank 2
    // {'.', '.', '.', 'P', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 8

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    printf("\n[DEBUG] Pawn structure score: %d\n", score);
    printf("[DEBUG] Expected pawn structure score: 236\n");
    /*
     * --- Calculation ---
     * game_phase = 0. Tapered weights: mg=0, eg=24. Final score is eg_eval.
     * White Scores:
     *   P on d3:
     *     eg = material value + pst bonus + doubled pawn penalty + isolated pawn penalty + passed pawns bonus
     *        = 120 + 10 - 20 - 20 + 20 * 2 = 130
     *   P on d2:
     *     eg = material value + pst bonus + doubled pawn penalty + isolated pawn penalty + passed pawns bonus 
     *        = 120 + 6 - 20 - 20 + 20 * 1 = 106
     *   Total White: eg=130+106=236
     * Black Scores:
     *   Total Black: eg=0
     * Net Scores: eg_eval=236-0=236.
     * Final Score = 236.
     */
    CU_ASSERT_EQUAL(score, 236);
}

static void test_evaluate_rook_on_seventh_and_open_file(void) {
    Pos p;
    // Position to test the rook on 7th rank bonus.
    // White's rook is on e7. The file is closed by pawns on e2 and e6 to ensure
    // only the 7th rank bonus is applied, not an open file bonus.
    // Material is equal (R+2P vs r+2p).
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', 'R', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'.', '.', '.', '.', 'R', '.', '.', '.'}  // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 8

    for (int r = 0; r < 8; r++) { memcpy(&p.b[(7 - r) * 8], board_visual[r], 8); }
    p.white_to_move = 1; p.castling = 0; p.ep = -1;

    int score = evaluate(&p);
    printf("\n[DEBUG] Rook on 7th score: %d\n", score);
    printf("[DEBUG] Expected rook on 7th score: -24\n");
    /* --- Calculation ---
     * game_phase = 1. Tapered weights: mg=1, eg=23.
     * White Scores:
     *  R on e7: 
     *      mg = material value + pst bonus + seventh rank bonus + open file bonus
     *         = 500 + 80 + 30 + 25 = 635
     *      eg = material value + pst bonus + seventh rank bonus + open file bonus
     *         = 500 - 3 + 30 + 25 = 552
     *  Total White: mg = 635, eg = 552
     * Final Score = (635*1 + 552*23)/24 = 555.
     */
    CU_ASSERT_EQUAL(score, -24);
    CU_ASSERT_EQUAL(score, -49);
}

int main(void) {
    CU_pSuite pSuite = NULL;

    // Initialize the CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite to the registry
    pSuite = CU_add_suite("EngineTestSuite", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add the tests to the suite
    if ((NULL == CU_add_test(pSuite, "test_evaluate_startpos", test_evaluate_startpos)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_material_advantage", test_evaluate_material_advantage)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_positional_advantage", test_evaluate_positional_advantage)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_pawn_structure", test_evaluate_pawn_structure)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_rook_on_seventh", test_evaluate_rook_on_seventh_and_open_file)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the CUnit Basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    unsigned int failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    // Return a failure code if any tests failed
    return (failures > 0) ? 1 : 0;
}