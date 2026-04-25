#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/engine.h"
#include "../src/movegen.h"
#include "../src/types.h"

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

/**
 * @brief Helper to check if a specific move exists in a list of moves.
 */
static int move_exists(const Move* moves, int n, int from, int to, char promo) {
    for (int i = 0; i < n; i++) {
        if (moves[i].from == from && moves[i].to == to && moves[i].promo == promo) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Helper to print all moves in a list for debugging, using algebraic notation.
 */
static void print_moves(const Move* moves, int n) {
    char from_sq[3], to_sq[3];
    for (int i = 0; i < n; i++) {
        index_to_sq(moves[i].from, from_sq);
        index_to_sq(moves[i].to, to_sq);
        printf("\t\t\t[DEBUG] Move %d: from %s to %s promo '%c'\n", i + 1, from_sq, to_sq, moves[i].promo);
    }
}

/**
 * @brief Tests pawn's initial single and double push.
 */
static void test_gen_pawn_initial_push(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', 'p', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', 'P', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[12] = 'P'; // White pawn on e2
    p.b[52] = 'p'; // Black pawn on e7

    Move moves[16];
    int n = 0;

    printf("\n\t[DEBUG] Testing white pawn initial push.\n");
    p.white_to_move = 1;
    gen_pawn(&p, 12, p.white_to_move, moves, &n);
    CU_ASSERT_EQUAL(n, 2);
    printf("\n\t\t[DEBUG] Number of moves for white pawn on e2: %d", n);
    printf("\n\t\t[DEBUG] Expected number of moves for white pawn on e2: 2\n");
    print_moves(moves, n);

    CU_ASSERT_TRUE(move_exists(moves, n, 12, 20, 0)); // e2e3
    CU_ASSERT_TRUE(move_exists(moves, n, 12, 28, 0)); // e2e4

    n = 0;
    printf("\n\t[DEBUG] Testing black pawn initial push.\n");
    p.white_to_move = 0;
    gen_pawn(&p, 52, p.white_to_move, moves, &n);
    CU_ASSERT_EQUAL(n, 2);
    printf("\n\t\t[DEBUG] Number of moves for black pawn on e7: %d", n);
    printf("\n\t\t[DEBUG] Expected number of moves for black pawn on e7: 2\n");
    print_moves(moves, n);
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 44, 0)); // e7e6
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 36, 0)); // e7e5
}

/**
 * @brief Tests pawn captures and blocked pushes.
 * NOTE: The current gen_pawn implementation does not generate en-passant captures.
 * This is a potential area for future improvement.
 */
static void test_gen_pawn_captures_and_blocked(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', 'p', 'n', 'p', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', 'P', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1
    
    p.b[28] = 'P'; // White pawn on e4
    p.b[35] = 'p'; // Black pawn on d5
    p.b[37] = 'p'; // Black pawn on f5
    p.b[36] = 'n'; // Black knight on e5, blocking the push

    Move moves[16];
    int n = 0;

    printf("\n\t[DEBUG] Testing pawn captures and blocked push.\n");
    p.white_to_move = 1;
    gen_pawn(&p, 28, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for white pawn on e4: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for white pawn on e4: 2\n");

    CU_ASSERT_EQUAL(n, 2);
    CU_ASSERT_TRUE(move_exists(moves, n, 28, 35, 0)); // e4xd5
    CU_ASSERT_TRUE(move_exists(moves, n, 28, 37, 0)); // e4xf5
    CU_ASSERT_FALSE(move_exists(moves, n, 28, 36, 0)); // e4e5 is blocked
    print_moves(moves, n);
}

/**
 * @brief Tests pawn promotion moves (push and capture).
 */
static void test_gen_pawn_promotion(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', 'r', 'r', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', 'P', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[52] = 'P'; // White pawn on e7
    p.b[60] = 'r'; // Black rook on e8 (blocks push)
    p.b[59] = 'r'; // Black rook on d8 (can be captured)

    Move moves[16];
    int n = 0;

    printf("\n\t[DEBUG] Testing pawn promotion captures.\n");
    p.white_to_move = 1;
    gen_pawn(&p, 52, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for white pawn on e7: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for white pawn on e7: 4\n");

    CU_ASSERT_EQUAL(n, 4); // Only 4 capture-promotions are possible
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 59, 'q'));
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 59, 'r'));
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 59, 'b'));
    CU_ASSERT_TRUE(move_exists(moves, n, 52, 59, 'n'));
    print_moves(moves, n);
}

/**
 * @brief Tests knight moves from center, edge, and with obstructions.
 */
static void test_gen_knight(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', 'N', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', 'p', '.', '.'}, // Rank 3
    // {'.', '.', 'P', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[27] = 'N'; // White knight on d4
    p.b[10] = 'P'; // Friendly pawn on c2 (blocks move)
    p.b[21] = 'p'; // Enemy pawn on f3 (can be captured)

    Move moves[16];
    int n = 0;

    printf("\n\t[DEBUG] Testing knight moves with blocks and captures.\n");
    p.white_to_move = 1;
    gen_knight(&p, 27, p.white_to_move, moves, &n);
    CU_ASSERT_EQUAL(n, 7);

    printf("\n\t\t[DEBUG] Number of moves for knight on d4: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for knight on d4: 7\n");
    print_moves(moves, n);

    CU_ASSERT_FALSE(move_exists(moves, n, 27, 10, 0)); // d4c2 is blocked
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 21, 0));  // d4xf3 is a capture
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 12, 0));  // d4e2
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 17, 0));  // d4b3
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 33, 0));  // d4b5
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 37, 0));  // d4f5
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 42, 0));  // d4c6
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 44, 0));  // d4e6
}

/**
 * @brief Tests slider moves for a rook.
 */
static void test_gen_slider_rook(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', 'R', '.', 'P', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', 'p', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[27] = 'R'; // White rook on d4
    p.b[29] = 'P'; // Friendly pawn on f4 (blocks)
    p.b[11] = 'p'; // Enemy pawn on d2 (can be captured)

    Move moves[32];
    int n = 0;

    printf("\n\t[DEBUG] Testing rook moves (via gen_slider).\n");
    p.white_to_move = 1;
    static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    gen_rook(&p, 27, p.white_to_move, d, 4, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for rook on d4: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for rook on d4: 10\n");

    print_moves(moves, n);

    // Up: d5,d6,d7,d8 (4)
    // Down: d3, d2(capture) (2)
    // Left: c4,b4,a4 (3)
    // Right: e4 (1)
    CU_ASSERT_EQUAL(n, 10);
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 11, 0)); // d4xd2
    CU_ASSERT_FALSE(move_exists(moves, n, 27, 29, 0)); // d4f4 is blocked
    CU_ASSERT_FALSE(move_exists(moves, n, 27, 30, 0)); // d4g4 is behind blocker
}

/**
 * @brief Tests slider moves for a bishop.
 */
static void test_gen_slider_bishop(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', 'P', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', 'B', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', 'p', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[27] = 'B'; // White bishop on d4
    p.b[45] = 'P'; // Friendly pawn on f6 (blocks)
    p.b[9] = 'p';  // Enemy pawn on b2 (can be captured)

    Move moves[32];
    int n = 0;

    printf("\n\t[DEBUG] Testing bishop moves (via gen_slider).\n");
    p.white_to_move = 1;
    static const int d[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    gen_bishop(&p, 27, p.white_to_move, d, 4, moves, &n);

    print_moves(moves, n);
    printf("\n\t\t[DEBUG] Number of moves for bishop on d4: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for bishop on d4: 9\n");

    // Up-Right: e5 (1)
    // Up-Left: c5,b6,a7 (3)
    // Down-Right: e3,f2,g1 (3)
    // Down-Left: c3,b2(capture) (2)
    CU_ASSERT_EQUAL(n, 9);
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 9, 0)); // d4xb2
    CU_ASSERT_FALSE(move_exists(moves, n, 27, 45, 0)); // d4f6 is blocked
    CU_ASSERT_FALSE(move_exists(moves, n, 27, 54, 0)); // d4g7 is behind blocker
}

/**
 * @brief Tests standard king moves.
 */
static void test_gen_king_standard(void) {
    Pos p;
    memset(p.b, '.', sizeof(p.b));
    p.castling = 0;
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', 'p', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', 'K', 'P', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[27] = 'K'; // White king on d4
    p.b[28] = 'P'; // Friendly pawn on e4 (blocks)
    p.b[35] = 'p'; // Enemy pawn on d5 (can be captured)

    Move moves[16];
    int n = 0;

    printf("\n\t[DEBUG] Testing standard king moves.\n");
    p.white_to_move = 1;
    gen_king(&p, 27, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on d4: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on d4: 7\n");
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 7);
    CU_ASSERT_TRUE(move_exists(moves, n, 27, 35, 0)); // d4xd5
    CU_ASSERT_FALSE(move_exists(moves, n, 27, 28, 0)); // d4e4 is blocked
}

/**
 * @brief Tests king castling move generation.
 */
static void test_gen_king_castling(void) {
    Pos p;
    Move moves[16];
    int n;

    // --- Test 1: White Kingside Castling ---
    printf("\n\t[DEBUG] Testing white kingside castling.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', 'K', '.', '.', 'R'}  // Rank 1

    p.b[4] = 'K'; p.b[7] = 'R'; p.white_to_move = 1; p.castling = 1; // WK rights
    n = 0;
    gen_king(&p, 4, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e1: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e1: 6\n");
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 6);
    CU_ASSERT_TRUE(move_exists(moves, n, 4, 6, 0)); // e1g1

    // --- Test 2: White Queenside Castling ---
    printf("\n\t[DEBUG] Testing white queenside castling.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'R', '.', '.', '.', 'K', '.', '.', '.'}  // Rank 1

    p.b[4] = 'K'; p.b[0] = 'R'; p.white_to_move = 1; p.castling = 2; // WQ rights
    n = 0;
    gen_king(&p, 4, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e1: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e1: 6\n");
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 6);
    CU_ASSERT_TRUE(move_exists(moves, n, 4, 2, 0)); // e1c1

    // --- Test 3: Black Kingside Castling ---
    printf("\n\t[DEBUG] Testing black kingside castling.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', 'k', '.', '.', 'r'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', '.', '.', '.', '.'}  // Rank 1

    p.b[60] = 'k'; p.b[63] = 'r'; p.white_to_move = 0; p.castling = 4; // BK rights
    n = 0;
    gen_king(&p, 60, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e8: %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e8: 6\n");
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 6);
    CU_ASSERT_TRUE(move_exists(moves, n, 60, 62, 0)); // e8g8

    // --- Test 4: Blocked by a piece ---
    printf("\n\t[DEBUG] Testing castling blocked by a piece.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', 'K', 'B', '.', 'R'}  // Rank 1

    p.b[4] = 'K'; p.b[7] = 'R'; p.b[5] = 'B'; // Bishop on f1
    p.white_to_move = 1; p.castling = 1;
    n = 0;
    gen_king(&p, 4, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e1 (blocked): %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e1 (blocked): 4\n");
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 4);
    CU_ASSERT_FALSE(move_exists(moves, n, 4, 6, 0));

    // --- Test 5: Blocked by an attacked square ---
    printf("\n\t[DEBUG] Testing castling blocked by an attack.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', '.', 'r', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', 'K', '.', '.', 'R'}  // Rank 1

    p.b[4] = 'K'; p.b[7] = 'R'; p.b[61] = 'r'; // Black rook attacks f1
    p.white_to_move = 1; p.castling = 1;
    n = 0;
    gen_king(&p, 4, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e1 (path attacked): %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e1 (path attacked): 4\n");
    print_moves(moves, n);
    // Note: The king should still have its normal moves, including the move upwards that keeps the king in check
    // However, the engine checks the legality of castling moves in the move generation function


    CU_ASSERT_EQUAL(n, 5); // 5 normal moves, but castling should be blocked
    CU_ASSERT_FALSE(move_exists(moves, n, 4, 6, 0));

    // --- Test 6: Blocked by being in check ---
    printf("\n\t[DEBUG] Testing castling while in check.\n");
    memset(p.b, '.', sizeof(p.b));
    p.ep = -1;
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 8
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 7
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 6
    // {'.', '.', '.', '.', 'r', '.', '.', '.'}, // Rank 5
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 4
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 3
    // {'.', '.', '.', '.', '.', '.', '.', '.'}, // Rank 2
    // {'.', '.', '.', '.', 'K', '.', '.', 'R'}  // Rank 1

    p.b[4] = 'K'; p.b[7] = 'R'; p.b[36] = 'r'; // Black rook attacks e1
    p.white_to_move = 1; p.castling = 1;
    n = 0;
    gen_king(&p, 4, p.white_to_move, moves, &n);

    printf("\n\t\t[DEBUG] Number of moves for king on e1 (in check): %d\n", n);
    printf("\t\t[DEBUG] Expected number of moves for king on e1 (in check): 5\n");
    // Note: The king should still have its normal moves, including the move upwards that keeps the king in check
    // However, the engine checks the legality of castling moves in the move generation function
    print_moves(moves, n);

    CU_ASSERT_EQUAL(n, 5);
    CU_ASSERT_FALSE(move_exists(moves, n, 4, 6, 0));
}


int main(void) {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    pSuite = CU_add_suite("MovegenTestSuite", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((NULL == CU_add_test(pSuite, "test_gen_pawn_initial_push", test_gen_pawn_initial_push)) ||
        (NULL == CU_add_test(pSuite, "test_gen_pawn_captures_and_blocked", test_gen_pawn_captures_and_blocked)) ||
        (NULL == CU_add_test(pSuite, "test_gen_pawn_promotion", test_gen_pawn_promotion)) ||
        (NULL == CU_add_test(pSuite, "test_gen_knight", test_gen_knight)) ||
        (NULL == CU_add_test(pSuite, "test_gen_slider_rook", test_gen_slider_rook)) ||
        (NULL == CU_add_test(pSuite, "test_gen_slider_bishop", test_gen_slider_bishop)) ||
        (NULL == CU_add_test(pSuite, "test_gen_king_standard", test_gen_king_standard)) ||
        (NULL == CU_add_test(pSuite, "test_gen_king_castling", test_gen_king_castling))
        ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    unsigned int failures = CU_get_number_of_failures();
    CU_cleanup_registry();

    return (failures > 0) ? 1 : 0;
}