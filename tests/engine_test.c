#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We need to include the headers for the functions we want to test.
#include "../src/engine.h"
#include "../src/algorithm.h"

/*
 * Test Suite setup and teardown functions.
 * These can be used to initialize and clean up resources for the tests.
 * For now, they are empty.
 */
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

/**
 * @brief Tests if the pos_from_fen function correctly parses the starting position.
 */
static void test_pos_from_fen_startpos(void) {
    Pos p;
    pos_from_fen(&p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Assert that the state is correct after loading the FEN
    CU_ASSERT_EQUAL(p.white_to_move, 1);
    CU_ASSERT_EQUAL(p.castling, 1 | 2 | 4 | 8); // All castling rights
    CU_ASSERT_EQUAL(p.ep, -1);
    // Assertions for piece positions
    CU_ASSERT_EQUAL(p.b[0], 'R');   // White rook on a1
    CU_ASSERT_EQUAL(p.b[4], 'K');   // White king on e1
    CU_ASSERT_EQUAL(p.b[63], 'r');  // Black rook on h8
    CU_ASSERT_EQUAL(p.b[59], 'q');  // Black queen on d8
}

/**
 * @brief Tests if make_move correctly handles a rook move (a1a4).
 */
static void test_make_move(void) {
    Pos p;

    // Manually create the board state.
    memset(p.b, '.', 64); // Start with an empty board
    p.b[0] = 'R';         // Place a white rook on a1
    p.b[4] = 'K';         // Place a white king on e1
    p.b[60] = 'k';        // Place a black king on e8

    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;

    Move m = {0, 24, 0}; // Move the rook from a1 to a4
    Pos np = make_move(&p, m);

    CU_ASSERT_EQUAL(np.b[0], '.');
    CU_ASSERT_EQUAL(np.b[24], 'R');
    CU_ASSERT_EQUAL(np.white_to_move, 0);
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
    if ((NULL == CU_add_test(pSuite, "test_pos_from_fen_startpos", test_pos_from_fen_startpos)) ||
        (NULL == CU_add_test(pSuite, "test_make_move_isolated", test_make_move)))
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
