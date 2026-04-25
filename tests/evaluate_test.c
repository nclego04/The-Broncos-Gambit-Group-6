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
    printf("\n\t[DEBUG] Startpos score: %d\n", score);
    /* The starting position is equal (0). This is the same from Black's perspective. */
    CU_ASSERT_EQUAL(score, 0);
}

static void test_evaluate_material_and_positional(void) {
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
    printf("\n\t[DEBUG] Score: %d\n", score); 
    printf("\t[DEBUG] Expected score: 298\n");
    /* Score reflects the knight's value, with minimal adjustment from PSTs.
     * game_phase=1 (one knight)
     * White N on e2 (sq 12 -> pst_sq 52): mg=-1, eg=-2
     * mg_eval = material value + pst bonus = 320 - 1 = 319
     * eg_eval = material value + pst bonus = 300 - 2 = 298
     * score = (mg_eval * 1 + eg_eval * 23) / 24 = (319 + 6854) / 24 = 7173 / 24 = 298.
     */
    CU_ASSERT_EQUAL(score, 298);
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
    printf("\n\t[DEBUG] Pawn structure score: %d\n", score);
    printf("\t[DEBUG] Expected pawn structure score: 236\n");
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

static void test_evaluate_rook_placement(void) {
    Pos p;
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', 'r', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };
    //
    // Board from Black's perspective:
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', 'r', '.', '.', '.'}  // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 6
    // {'.', '.', '.', '.', '.', '.', 'r', '.'}  // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 8

    for (int r = 0; r < 8; r++) { memcpy(&p.b[(7 - r) * 8], board_visual[r], 8); }
    p.white_to_move = 1; p.castling = 0; p.ep = -1;

    int score = evaluate(&p);
    printf("\n\t[DEBUG] Rook score: %d\n", score);
    printf("\t[DEBUG] Expected rook score: -564\n");
    /* --- Calculation ---
     * game_phase = 2. Tapered weights: mg=2, eg=22.
     * Black Score:
     *  R on g7: 
     *      mg = material value + pst bonus + seventh rank bonus + open file bonus
     *         = 500 + 26 + 30 + 25 = 581
     *      eg = material value + pst bonus + seventh rank bonus + open file bonus
     *         = 500 + 8 + 30 + 25 = 563
     *  Total Black: mg = 581, eg = 563
     * Final Score = -(581*2 + 563*22)/24 = -564.
     */
    CU_ASSERT_EQUAL(score, -564);
}

static void test_evaluate_bishop_pair(void) {
    Pos p;
    // Position to test bishop pair by using the main evaluate function.
    // Black's king on g8 has a full pawn shield.
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'B', 'B', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };

        // Board from Black's perspective:
    // {'B', 'B', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 2
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
    printf("\n\t[DEBUG] Bishop pair score: %d\n", score);
    printf("\t[DEBUG] Expected bishop pair score: 676\n");
        /* --- Calculation ---
     * game_phase = 2. Tapered weights: mg=2, eg=22.
     * White Score:
     *  B on a8: 
     *      mg = material value + pst bonus 
     *         = 330 - 33 = 297
     *      eg = material value + pst bonus
     *         = 330 - 23 = 307
     *  B on b8: 
     *     mg = material value + pst bonus 
     *        = 330 - 3 = 327
     *     eg = material value + pst bonus
     *        = 330 - 9 = 321
     * Total White: 
     *     mg = 297 + 327 + bishop_pair_bonus = 297 + 327 + 40 = 664
     *     eg = 307 + 321 + bishop_pair_bonus = 307 + 321 + 50 = 678
     * Final Score = (664*2 + 678*22)/24 = 676.
     */
    CU_ASSERT_EQUAL(score, 676);
}

static void test_evaluate_missing_king(void) {
    Pos p;
    // Edge Case: An illegal board with White's king missing.
    // The evaluation should be extremely negative, reflecting the loss of the king.
    const char board_visual[8][8] = { // Board from White's perspective
        {'.', '.', '.', '.', 'k', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    };

        // Board from Black's perspective:
    // {'B', 'B', '.', '.', '.', '.', '.', '.'}  // Rank 1
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 2
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
    printf("\n\t[DEBUG] Check if score < -19,0000");    
    printf("\n\t[DEBUG] Missing king score: %d\n", score);
    /*
     * --- Analysis ---
     * The evaluation will be massively negative because White's score is missing the
     * 20,000 base value for the king.
     * The final score will be roughly 0 - 20000 = -20000.
     * We assert that the score is less than -19000 to confirm this catastrophic evaluation.
     * This demonstrates the evaluation function correctly reflects the devastating material loss,
     * even though the position is illegal.
     */
    CU_ASSERT(score < -19000);
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
        (NULL == CU_add_test(pSuite, "test_evaluate_material_and_positional", test_evaluate_material_and_positional)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_pawn_structure", test_evaluate_pawn_structure)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_rook_placement", test_evaluate_rook_placement)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_bishop_pair", test_evaluate_bishop_pair)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_missing_king", test_evaluate_missing_king)))
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