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
    const char board_visual[8][8] = {
        {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}, // Rank 8
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}, // Rank 2
        {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}  // Rank 1
    };

    // Copy the visual board into the engine's 1D representation.
    // The engine sees Rank 1 (from board_visual[7]) at p.b[0-7], Rank 2 at p.b[8-15], etc.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 1 | 2 | 4 | 8; // All rights: WK, WQ, BK, BQ
    p.ep = -1;

    int score = evaluate(&p);
    CU_ASSERT_EQUAL(score, 0); // Starting position should be evaluated as equal
}

static void test_evaluate_material_advantage(void) {
    Pos p;

    // Position to test pure material advantage: White Knight vs Black lone King.
    // All pawn structures and other positional factors are neutralized.
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', 'k', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', 'N', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', 'K', '.', '.', '.'}  // Rank 1
    };

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    // Score should reflect the knight's value, adjusted by the game phase.
    // game_phase=1, score = (mg_knight * 1 + eg_knight * 23) / 24 + pst
    // score = ( (320+20)*1 + (300+20)*23 ) / 24 = (340 + 7360) / 24 = 7700/24 = 320
    CU_ASSERT_EQUAL(score, 320);
}

static void test_evaluate_positional_advantage(void) {
    Pos p;

    // Position to test pure positional advantage:
    // Material is equal (K+N vs k+n).
    // White's knight is centralized on d4 (good), Black's is on the rim at h8 (bad).
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', 'k', '.', '.', 'n'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', 'N', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', 'K', '.', '.', '.'}  // Rank 1
    };

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    // The score should reflect the difference in the knights' positional values
    // from the Piece-Square Tables.
    // White N on d4 (pst_sq 35) = +20
    // Black n on h8 (pst_sq 63) = -50
    // Difference = 20 - (-50) = 70.
    // This difference is constant for both middlegame and endgame, so the tapered eval is also 70.
    CU_ASSERT_EQUAL(score, 70);
}

static void test_evaluate_doubled_pawns(void) {
    Pos p;

    // Position designed to test the doubled pawn penalty.
    // - Material is equal (3 pawns vs 3 pawns).
    // - Isolated pawn penalties are avoided by placing other pawns on adjacent files.
    // - Piece-Square Table (PST) scores for both sides are constructed to be identical.
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', 'k', '.', '.', '.'}, // Rank 8
        {'.', '.', 'p', 'p', '.', '.', '.', '.'}, // Rank 7 (Black pawns on c7, d7)
        {'.', '.', '.', '.', 'p', '.', '.', '.'}, // Rank 6 (Black pawn on e6)
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
        {'.', '.', '.', 'P', '.', '.', '.', '.'}, // Rank 3 (White pawn on d3)
        {'.', '.', 'P', 'P', '.', '.', '.', '.'}, // Rank 2 (White pawns on c2, d2)
        {'.', '.', '.', '.', 'K', '.', '.', '.'}  // Rank 1
    };

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    // The game phase is 0 (endgame).
    // The only scoring difference comes from White's doubled pawns on the d-file.
    // The penalty is applied to each of the two pawns.
    // White's score = (base) + (pst) - 40 (doubled)
    // Black's score = (base) + (pst) - 0
    // Final score = White's score - Black's score = -40.
    CU_ASSERT_EQUAL(score, -40);
}

static void test_evaluate_isolated_pawns(void) {
    Pos p;

    // Position to test isolated pawn penalty.
    // Material and piece positions are symmetrical.
    // White's pawns are connected; Black's pawns on c5 and e5 are isolated.
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', 'k', '.', '.', '.'}, // Rank 8
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
        {'.', '.', 'p', '.', 'p', '.', '.', '.'}, // Rank 5 (Black pawns on c5, e5)
        {'.', '.', 'P', 'P', '.', '.', '.', '.'}, // Rank 4 (White pawns on c4, d4)
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
        {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
        {'.', '.', '.', '.', 'K', '.', '.', '.'}  // Rank 1
    };

    // Copy the visual board into the engine's 1D representation.
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }

    p.white_to_move = 1;
    p.castling = 0; // No castling rights
    p.ep = -1;

    int score = evaluate(&p);
    // The game phase is 0 (endgame).
    // The score should reflect Black's isolated pawn penalties.
    // Black's pawns on c5 and e5 each get a -20 penalty = -40 total for Black.
    // White has no isolated pawns.
    // PST scores are symmetrical and cancel out.
    // Final score = White's penalties (0) - Black's penalties (-40) = 40.
    CU_ASSERT_EQUAL(score, 40);
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
        (NULL == CU_add_test(pSuite, "test_evaluate_doubled_pawns", test_evaluate_doubled_pawns)) ||
        (NULL == CU_add_test(pSuite, "test_evaluate_isolated_pawns", test_evaluate_isolated_pawns)))
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