#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We need to include the headers for the functions we want to test.
#include "../src/engine.h"
#include "../src/algorithm.h"
#include "../src/types.h"

/*
 * Test Suite setup and teardown functions.
 * These can be used to initialize and clean up resources for the tests.
 * For now, they are empty.
 */
int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

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
 * @brief Tests if the pos_from_fen function correctly parses the starting position.
 */
static void test_pos_from_fen_startpos(void) {
    Pos p;
    pos_from_fen(&p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Assert that the state is correct after loading the FEN
    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t[DEBUG] Check if it is White's turn to move: %d\n", p.white_to_move);
    printf("\t[DEBUG] Expected: 1 (White to move)\n");

    CU_ASSERT_EQUAL(p.castling, 1 | 2 | 4 | 8); // All castling rights
    printf("\n\t[DEBUG] Check castling rights bitmask: %d\n", p.castling);
    printf("\t[DEBUG] Expected: 15 (All castling rights)\n");

    CU_ASSERT_EQUAL(p.ep, -1);

    // Assertions for piece positions
    CU_ASSERT_EQUAL(p.b[0], 'R');   // White rook on a1
    printf("\n\t[DEBUG] Check piece on a1: %c\n", p.b[0]);
    printf("\t[DEBUG] Expected: R (White rook on a1)\n");

    CU_ASSERT_EQUAL(p.b[4], 'K');   // White king on e1
    printf("\n\t[DEBUG] Check piece on e1: %c\n", p.b[4]);
    printf("\t[DEBUG] Expected: K (White king on e1)\n");

    CU_ASSERT_EQUAL(p.b[63], 'r');  // Black rook on h8
    printf("\n\t[DEBUG] Check piece on h8: %c\n", p.b[63]);
    printf("\t[DEBUG] Expected: r (Black rook on h8)\n");

    CU_ASSERT_EQUAL(p.b[59], 'q');  // Black queen on d8
    printf("\t[DEBUG] Check piece on d8: %c\n", p.b[59]);
    printf("\t[DEBUG] Expected: q (Black queen on d8)\n");
}

/**
 * @brief Tests pos_from_fen with a truncated FEN string.
 */
static void test_fen_incomplete(void) {
    Pos p;
    // This FEN is missing castling, EP, and clock fields.
    printf("\n\t[DEBUG] Testing incomplete FEN with missing castling.");
    pos_from_fen(&p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");
    
    // The parser should default to no castling rights.
    CU_ASSERT_EQUAL(p.castling, 0);
    printf("\n\t\t[DEBUG] Check castling rights bitmask: %d\n", p.castling);
    printf("\t\t[DEBUG] Expected: 0 (No castling rights)\n");

    // The parser should default to no en passant square.
    CU_ASSERT_EQUAL(p.ep, -1);
}

/**
 * @brief Tests pos_from_fen with a malformed piece placement string.
 */
static void test_fen_malformed_placement(void) {
    Pos p;
    // This FEN has a malformed rank (7 pawns instead of 8 squares worth).
    printf("\n\t[DEBUG] Testing malformed placement.");
    pos_from_fen(&p, "rnbqkbnr/ppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    // The parser should leave the last square of the rank empty.
    CU_ASSERT_EQUAL(p.b[55], '.');
    printf("\n\t\t[DEBUG] Piece on h7: %c\n", p.b[55]);
    printf("\t\t[DEBUG] Expected: . (h7 should be empty)\n");
}

/**
 * @brief Tests pos_from_fen with an invalid side-to-move character.
 */
static void test_fen_invalid_stm(void) {
    Pos p;
    // This FEN has 'z' as the side to move.
    printf("\n\t[DEBUG] Testing invalid side-to-move character.");
    pos_from_fen(&p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR z KQkq - 0 1");
    
    // The parser should default to black to move if the character is not 'w'.
    CU_ASSERT_EQUAL(p.white_to_move, 0);
    printf("\n\t\t[DEBUG] Check if it is Black's turn to move: %d\n", p.white_to_move);
    printf("\t\t[DEBUG] Expected: 0 (Black to move)\n");
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
    
    printf("\n\t[DEBUG] Initial piece on a1: %c\n", p.b[0]);
    printf("\t[DEBUG] Expected: R (White rook on a1)\n");

    printf("\n\t[DEBUG] White to move: %d\n", p.white_to_move);
    printf("\t[DEBUG] Expected: 1 (White to move)\n");

    Move m = {0, 24, 0}; // Move the rook from a1 to a4
    Pos np = make_move(&p, m);

    // Assertions to verify the move was made correctly
    CU_ASSERT_EQUAL(np.b[24], 'R');
    printf("\n\t[DEBUG] New piece on a4: %c\n", np.b[24]);
    printf("\t[DEBUG] Expected: R (White rook on a4)\n");

    CU_ASSERT_EQUAL(np.b[0], '.');
    printf("\n\t[DEBUG] Old piece on a1: %c\n", np.b[0]);
    printf("\t[DEBUG] Expected: . (a1 should now be empty)\n");

    CU_ASSERT_EQUAL(np.white_to_move, 0);
    printf("\n\t[DEBUG] White to move after move: %d\n", np.white_to_move);
    printf("\t[DEBUG] Expected: 0 (Black to move after White's move)\n");
}

/**
 * @brief Tests if make_move correctly handles pawn promotion.
 */
static void test_make_move_promotion(void) {
    Pos p;
    memset(p.b, '.', 64);
    p.b[52] = 'P'; // White pawn on e7
    p.b[4] = 'K';  // White king
    p.b[60] = 'k'; // Black king
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;

    Move m = {52, 60, 'q'}; // Move e7 to e8, promoting to a Queen
    Pos np = make_move(&p, m);

    printf("\n\t[DEBUG] Testing pawn promotion to Queen.");
    CU_ASSERT_EQUAL(np.b[60], 'Q');
    printf("\n\t\t[DEBUG] Piece on e8 after promotion: %c\n", np.b[60]);
    printf("\t\t[DEBUG] Expected: Q\n");

    CU_ASSERT_EQUAL(np.b[52], '.');
    printf("\n\t\t[DEBUG] Piece on e7 after promotion: %c\n", np.b[52]);
    printf("\t\t[DEBUG] Expected: .\n");
}

/**
 * @brief Tests if make_move correctly handles castling.
 */
static void test_make_move_castling(void) {
    Pos p;
    // Set up a position where White can castle kingside
    memset(p.b, '.', 64);
    p.b[4] = 'K';
    p.b[7] = 'R';
    p.b[60] = 'k';
    p.white_to_move = 1;
    p.castling = 1; // White kingside castling right
    p.ep = -1;

    Move m = {4, 6, 0}; // White castles kingside (e1g1)
    Pos np = make_move(&p, m);

    printf("\n\t[DEBUG] Testing kingside castling.");
    CU_ASSERT_EQUAL(np.b[6], 'K');
    printf("\n\t\t[DEBUG] Piece on g1 after castling: %c\n", np.b[6]);
    printf("\t\t[DEBUG] Expected: K\n");

    CU_ASSERT_EQUAL(np.b[5], 'R');
    printf("\n\t\t[DEBUG] Piece on f1 after castling: %c\n", np.b[5]);
    printf("\t\t[DEBUG] Expected: R\n");
}

/**
 * @brief Tests if make_move correctly updates castling rights after a king move.
 */
static void test_make_move_castling_rights(void) {
    Pos p;
    // Manually set up the starting position to have all castling rights.
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
        memcpy(&p.b[(7 - r) * 8], startpos_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 1 | 2 | 4 | 8; // All rights (15)
    printf("\n\t[DEBUG] Initial castling rights: %d (1111b)\n", p.castling);
    p.ep = -1;

    Move m = {4, 5, 0}; // King moves e1-f1
    Pos np = make_move(&p, m);

    // White moving the king should remove both white castling rights (WK=1, WQ=2).
    // The `make_move` function correctly does this with `np.castling &= ~3`.
    // Initial rights: 15 (1111b). After move: 15 & ~3 = 12 (1100b).
    printf("\n\t[DEBUG] Testing castling rights update after king move.");
    CU_ASSERT_EQUAL(np.castling, 12);
    printf("\n\t\t[DEBUG] Castling rights after king move: %d (1100b)\n", np.castling);
    printf("\t\t[DEBUG] Expected: 12 (1100b)\n");
}

/**
 * @brief Tests how make_move handles an illegal piece movement (e.g., a rook moving diagonally).
 */
static void test_make_move_illegal_movement(void) {
    Pos p;
    // Set up a simple position
    memset(p.b, '.', 64);
    p.b[0] = 'R';  // White rook on a1
    p.b[4] = 'K';  // White king
    p.b[60] = 'k'; // Black king
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;

    // Define an illegal move for a rook (a1 to c3, diagonally)
    Move m = {0, 18, 0}; // from a1 (0) to c3 (18)
    Pos np = make_move(&p, m);

    // make_move does not validate legality; it should just execute the move.
    // This test confirms that the piece is "teleported" without checking rules.
    printf("\n\t[DEBUG] Testing illegal rook move (teleport).");
    CU_ASSERT_EQUAL(np.b[18], 'R');
    printf("\n\t\t[DEBUG] Piece on c3 after illegal move: %c\n", np.b[18]);
    printf("\t\t[DEBUG] Expected: R (White rook on c3)\n");

    CU_ASSERT_EQUAL(np.b[0], '.');
    printf("\n\t\t[DEBUG] Piece on a1 after illegal move: %c\n", np.b[0]);
    printf("\t\t[DEBUG] Expected: . (a1 should now be empty)\n");
}

/**
 * @brief Tests if make_move correctly updates castling rights after a king move.
 */
static void test_legal_moves(void) {
    Pos p;
    Move moves[256];
    int n;

    // Test 1: Starting position has 20 legal moves.
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
        memcpy(&p.b[(7 - r) * 8], startpos_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 1 | 2 | 4 | 8;
    p.ep = -1;
    n = legal_moves(&p, moves);

    CU_ASSERT_EQUAL(n, 20);
    printf("\n\t[DEBUG] Number of legal moves in start position: %d\n", n);
    printf("\t[DEBUG] Expected: 20\n");

    // Test 2: A complex middlegame position (Kiwipete FEN).
    // This position is a standard test for move generation.
    const char kiwipete_visual[8][8] = {
        {'r', '.', '.', '.', 'k', '.', '.', 'r'},
        {'p', '.', 'p', 'p', 'q', 'p', 'b', '.'},
        {'b', 'n', '.', '.', 'p', 'n', 'p', '.'},
        {'.', '.', '.', 'P', 'N', '.', '.', '.'},
        {'.', 'p', '.', '.', 'P', '.', '.', '.'},
        {'.', '.', 'N', '.', '.', 'Q', '.', 'p'},
        {'P', 'P', 'P', 'B', 'B', 'P', 'P', 'P'},
        {'R', '.', '.', '.', 'K', '.', '.', 'R'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], kiwipete_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 1 | 2 | 4 | 8;
    p.ep = -1;
    n = legal_moves(&p, moves);
    CU_ASSERT_EQUAL(n, 48);
    printf("\n\t[DEBUG] Number of legal moves in Kiwipete position: %d\n", n);
    printf("\t[DEBUG] Expected: 48\n");
}

/**
 * @brief Tests that a pinned piece has no legal moves.
 */
static void test_legal_moves_pinned_piece(void) {
    Pos p;
    Move moves[256];
    // Position: White Knight on d2 is pinned to the King on d1 by a Black Rook on d8.
    // The knight cannot move. Only the king has legal moves (4 of them).
    const char board_visual[8][8] = {
        {'.', '.', '.', 'r', '.', 'k', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', 'N', '.', '.', '.', '.'},
        {'.', '.', '.', 'K', '.', '.', '.', '.'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;
    int n = legal_moves(&p, moves);
    CU_ASSERT_EQUAL(n, 4);
    printf("\n\t[DEBUG] Legal moves for pinned piece position: %d\n", n);
    printf("\t[DEBUG] Expected: 4 (only king moves are legal)\n");
}

/**
 * @brief Tests that the only legal moves are ones that resolve a check.
 */
static void test_legal_moves_get_out_of_check(void) {
    Pos p;
    Move moves[256];
    // Position: White King on h1 is in check by a Queen on g1.
    // The only legal move is for the Rook on g2 to capture the queen.
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', 'R', '.'},
        {'.', '.', '.', '.', '.', 'k', 'q', 'K'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;
    int n = legal_moves(&p, moves);
    CU_ASSERT_EQUAL(n, 1);
    printf("\n\t[DEBUG] Legal moves when in check: %d\n", n);
    printf("\t[DEBUG] Expected: 1 (Rxg2)\n");

    CU_ASSERT_EQUAL(moves[0].from, 14); // g2
    printf("\n\t[DEBUG] Move from square index: %d\n", moves[0].from);
    printf("\t[DEBUG] Expected: 14 (g2)\n");

    CU_ASSERT_EQUAL(moves[0].to, 6);   // g1
    printf("\n\t[DEBUG] Move to square index: %d\n", moves[0].to);
    printf("\t[DEBUG] Expected: 6 (g1)\n");
}

/**
 * @brief Tests that there are no legal moves in a checkmate position.
 */
static void test_legal_moves_checkmate(void) {
    Pos p;
    Move moves[256];
    // Position: White is in checkmate (back-rank mate).
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', '.', '.', 'k', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', 'P', 'P'},
        {'.', '.', 'r', '.', '.', '.', '.', 'K'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 0;
    p.ep = -1;
    int n = legal_moves(&p, moves);
    CU_ASSERT_EQUAL(n, 0);
    printf("\n\t[DEBUG] Legal moves in checkmate: %d\n", n);
    printf("\t[DEBUG] Expected: 0\n");
}

/**
 * @brief Tests that there are no legal moves in a stalemate position.
 */
static void test_legal_moves_stalemate(void) {
    Pos p;
    Move moves[256];
    // Position: Black is in stalemate. The king is not in check but has no legal moves.
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', '.', '.', '.', 'k'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', 'Q', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'K', '.', '.', '.', '.', '.', '.', '.'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }
    p.white_to_move = 0;
    p.castling = 0;
    p.ep = -1;
    int n = legal_moves(&p, moves);
    CU_ASSERT_EQUAL(n, 0);
    printf("\n\t[DEBUG] Legal moves in stalemate: %d\n", n);
    printf("\t[DEBUG] Expected: 0\n");
}

/**
 * @brief Tests that castling is not a legal move when the king is in check.
 */
static void test_legal_moves_castling_in_check(void) {
    Pos p;
    Move moves[256];
    // Position: White is in check from a bishop on a6. Castling is not legal.
    // There are 5 legal moves to get out of check (d4, Bd3, Nd3, Qd3, Kf1).
    const char board_visual[8][8] = {
        {'.', '.', '.', '.', 'k', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', 'b', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', 'K', '.', '.', 'R'}
    };
    for (int r = 0; r < 8; r++) {
        memcpy(&p.b[(7 - r) * 8], board_visual[r], 8);
    }
    p.white_to_move = 1;
    p.castling = 1; // Give WK castling right
    p.ep = -1;
    int n = legal_moves(&p, moves);

    // print the legal moves
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("\t[DEBUG] Move %d: from %d to %d\n", i, moves[i].from, moves[i].to);
    }

    CU_ASSERT_EQUAL(n, 4);
    printf("\n\t[DEBUG] Legal moves when castling is blocked by check: %d\n", n);
    printf("\t[DEBUG] Expected: 2\n");

    // Explicitly check that no castling move was generated.
    int castling_move_found = 0;
    for (int i = 0; i < n; i++) {
        // White kingside castling is a move from e1 (4) to g1 (6)
        if (moves[i].from == 4 && moves[i].to == 6) {
            castling_move_found = 1;
            break;
        }
    }
    CU_ASSERT_EQUAL(castling_move_found, 0);
    printf("\n\t[DEBUG] Check if castling move was generated: %d\n", castling_move_found);
    printf("\t[DEBUG] Expected: 0 (No castling move should be found)\n");
}

/**
 * @brief Tests the parse_position function with "startpos" and a sequence of moves.
 */
static void test_parse_position_startpos_moves(void) {
    Pos p;
    // The command string sets up the start position, then plays e2e4 and e7e5.
    char cmd[] = "position startpos moves e2e4 e7e5";
    
    parse_position(&p, cmd);

    // After "e2e4 e7e5", it should be White's turn again.

    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t[DEBUG] White to move after moves: %d\n", p.white_to_move);
    printf("\t[DEBUG] Expected: 1 (White to move after e2e4 e7e5)\n");
    
    CU_ASSERT_EQUAL(p.b[12], '.'); // e2 is empty
    printf("\n\t[DEBUG] Piece on e2: %c\n", p.b[12]);
    printf("\t[DEBUG] Expected: . (e2 should be empty after e2e4)\n");
    
    CU_ASSERT_EQUAL(p.b[28], 'P'); // White pawn on e4
    printf("\n\t[DEBUG] Piece on e4: %c\n", p.b[28]);
    printf("\t[DEBUG] Expected: P (White pawn on e4)\n");

    CU_ASSERT_EQUAL(p.b[52], '.'); // e7 is empty
    printf("\n\t[DEBUG] Piece on e7: %c\n", p.b[52]);
    printf("\t[DEBUG] Expected: . (e7 should be empty after e7e5)\n");

    CU_ASSERT_EQUAL(p.b[36], 'p'); // Black pawn on e5
    printf("\n\t[DEBUG] Piece on e5: %c\n", p.b[36]);
    printf("\t[DEBUG] Expected: p (Black pawn on e5)\n");
}

/**
 * @brief Tests parse_position with a long sequence of moves from the start position.
 */
static void test_parse_position_long_move_list(void) {
    Pos p;
    // A common opening sequence (Ruy Lopez, Morphy Defense)
    char cmd[] = "position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7";
    
    parse_position(&p, cmd);

    // After 10 half-moves, it should be White's turn again.
    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t[DEBUG] White to move after long move list: %d\n", p.white_to_move);
    printf("\t[DEBUG] Expected: 1 (White to move)\n");
    
    // Check a few key pieces to verify the final position.
    // White castled, so King is on g1 and Rook is on f1.
    CU_ASSERT_EQUAL(p.b[6], 'K'); // White King on g1
    printf("\n\t[DEBUG] Piece on g1: %c\n", p.b[6]);
    printf("\t[DEBUG] Expected: K\n");

    CU_ASSERT_EQUAL(p.b[5], 'R'); // White Rook on f1
    printf("\n\t[DEBUG] Piece on f1: %c\n", p.b[5]);
    printf("\t[DEBUG] Expected: R\n");

    // Black developed knight and bishop.
    CU_ASSERT_EQUAL(p.b[45], 'n'); // Black Knight on f6
    printf("\n\t[DEBUG] Piece on f6: %c\n", p.b[45]);
    printf("\t[DEBUG] Expected: n\n");

    CU_ASSERT_EQUAL(p.b[52], 'b'); // Black Bishop on e7
    printf("\n\t[DEBUG] Piece on e7: %c\n", p.b[52]);
    printf("\t[DEBUG] Expected: b\n");

    // White's attacking bishop is on a4.
    CU_ASSERT_EQUAL(p.b[24], 'B'); // White Bishop on a4
    printf("\n\t[DEBUG] Piece on a4: %c\n", p.b[24]);
    printf("\t[DEBUG] Expected: B\n");
}

/**
 * @brief Tests parse_position with unusual whitespace.
 */
static void test_parse_position_whitespace(void) {
    Pos p;
    
    printf("\n\t[DEBUG] Testing parser with unusual whitespace.\n");
    
    // Test with multiple spaces and tabs between tokens.
    char cmd[] = "position   startpos\tmoves e2e4  e7e5";
    printf("\n\t[DEBUG] Command: %s\n", cmd);
    
    parse_position(&p, cmd);

    // Assert that the final position is correct despite the whitespace.
    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t[DEBUG] White to move after whitespace: %d\n", p.white_to_move);
    printf("\t[DEBUG] Expected: 1 (White to move)\n");

    CU_ASSERT_EQUAL(p.b[28], 'P'); // White pawn on e4
    printf("\n\t[DEBUG] Piece on e4: %c\n", p.b[28]);
    printf("\t[DEBUG] Expected: P (White pawn on e4)\n");


    CU_ASSERT_EQUAL(p.b[36], 'p'); // Black pawn on e5
    printf("\n\t[DEBUG] Piece on e5: %c\n", p.b[36]);
    printf("\t[DEBUG] Expected: p (Black pawn on e5)\n");
}

/**
 * @brief Tests parse_position with various incomplete commands.
 */
static void test_parse_position_incomplete_commands(void) {
    Pos p, start_pos;
    // Manually set up a known initial state to compare against.

    printf("\n\t[DEBUG] Testing incomplete 'position' command.\n");
    setup_start_pos(&start_pos);
    p = start_pos;
    parse_position(&p, "position");
    int are_equal_1 = (memcmp(&p, &start_pos, sizeof(Pos)) == 0);
    CU_ASSERT_TRUE(are_equal_1); // Should not change the state
    printf("\n\t\t[DEBUG] Compare states before and after parsing 'position' (1 is equal): %d\n", are_equal_1);
    printf("\t\t[DEBUG] Expected: 1.\n");

    printf("\n\t[DEBUG] Testing 'position startpos moves' with no moves.\n");
    memset(&p, 0, sizeof(Pos)); // Zero out p to ensure it's set by the function.
    parse_position(&p, "position startpos moves");
    int are_equal_2 = (memcmp(&p, &start_pos, sizeof(Pos)) == 0);
    CU_ASSERT_TRUE(are_equal_2); // Should be startpos
    printf("\n\t\t[DEBUG] Compare states after parsing 'position startpos moves' (1 is equal): %d\n", are_equal_2);
    printf("\t\t[DEBUG] Expected: 1.\n");

    printf("\n\t[DEBUG] Testing 'position fen' with no FEN string.\n");
    parse_position(&p, "position fen");
    // This should result in an empty board as per pos_from_fen implementation.
    int is_empty = 1;
    for (int i = 0; i < 64; i++) {
        if (p.b[i] != '.') {
            is_empty = 0;
            break;
        }
    }
    CU_ASSERT_TRUE(is_empty);
    printf("\n\t\t[DEBUG] Check if board is empty after 'position fen' with no FEN: %d\n", is_empty);
    printf("\t\t[DEBUG] Expected: 1 (Board should be empty)\n");
}

/**
 * @brief Tests parse_position with a malformed command where 'moves' is misplaced.
 */
static void test_parse_position_malformed_command(void) {
    Pos p, start_pos;
    setup_start_pos(&start_pos);
    p = start_pos;

    // "position moves e2e4" is malformed. The 'moves' keyword should be ignored
    // because it does not follow 'startpos' or 'fen'. The board state should not change.
    printf("\n\t[DEBUG] Testing malformed 'position moves ...' command.\n");
    parse_position(&p, "position moves e2e4");
    
    int are_equal = (memcmp(&p, &start_pos, sizeof(Pos)) == 0);
    CU_ASSERT_TRUE(are_equal);
    printf("\n\t\t[DEBUG] Compare states after malformed command (1 is equal): %d\n", are_equal);
    printf("\t\t[DEBUG] Expected: 1.\n");
}

/**
 * @brief Tests that the parser gracefully handles an invalid move string.
 */
static void test_parse_position_invalid_move(void) {
    Pos p;
    // The parser should gracefully skip the invalid move "garbage" and continue.
    char cmd[] = "position startpos moves e2e4 garbage e7e5";
    
    parse_position(&p, cmd);

    printf("\n\t[DEBUG] Testing parser with invalid move string 'position startpos moves e2e4 garbage e7e5'.\n");
    // Assert that both valid moves were applied.
    CU_ASSERT_EQUAL(p.b[28], 'P'); // White pawn on e4
    printf("\n\t[DEBUG] Piece on e4: %c\n", p.b[28]);
    printf("\t\t[DEBUG] Expected: P (White pawn on e4)\n");

    CU_ASSERT_EQUAL(p.b[36], 'p'); // Black pawn on e5
    printf("\n\t\t[DEBUG] Piece on e5: %c\n", p.b[36]);
    printf("\t\t[DEBUG] Expected: p (Black pawn on e5)\n");

    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t\t[DEBUG] White to move after invalid move string: %d\n", p.white_to_move);
    printf("\t\t[DEBUG] Expected: 1 (White to move after valid moves)\n");
}

/**
 * @brief Tests the parser's ability to handle a very long move list to check for overflows.
 */
static void test_parse_position_resource_limit(void) {
    Pos p;
    // Create a very long command string (800 half-moves).
    char *long_cmd = malloc(8192);
    CU_ASSERT_PTR_NOT_NULL(long_cmd);
    strcpy(long_cmd, "position startpos moves");
    // A simple, repetitive, and legal back-and-forth move sequence.
    for (int i = 0; i < 200; i++) {
        strcat(long_cmd, " b1c3 b8c6 c3b1 c6b8");
    }
    
    printf("\n\t[DEBUG] Testing parser with a loop of 'b1c3 b8c6 c3b1 c6b8'.\n");
    parse_position(&p, long_cmd);
    
    // After all moves, the knights should be back home and it should be white's turn.
    CU_ASSERT_EQUAL(p.b[1], 'N');   // White knight on b1
    printf("\n\t\t[DEBUG] Piece on b1: %c\n", p.b[1]);
    printf("\t\t[DEBUG] Expected: N\n");


    CU_ASSERT_EQUAL(p.b[57], 'n');  // Black knight on b8
    printf("\n\t\t[DEBUG] Piece on b8: %c\n", p.b[57]);
    printf("\t\t[DEBUG] Expected: n\n");

    CU_ASSERT_EQUAL(p.white_to_move, 1);
    printf("\n\t\t[DEBUG] White to move after long move list: %d\n", p.white_to_move);
    printf("\t\t[DEBUG] Expected: 1 (White to move)\n");

    free(long_cmd);
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
        (NULL == CU_add_test(pSuite, "test_fen_incomplete", test_fen_incomplete)) ||
        (NULL == CU_add_test(pSuite, "test_fen_malformed_placement", test_fen_malformed_placement)) ||
        (NULL == CU_add_test(pSuite, "test_fen_invalid_stm", test_fen_invalid_stm)) ||
        (NULL == CU_add_test(pSuite, "test_make_move", test_make_move)) ||
        (NULL == CU_add_test(pSuite, "test_make_move_promotion", test_make_move_promotion)) ||
        (NULL == CU_add_test(pSuite, "test_make_move_castling", test_make_move_castling)) ||
        (NULL == CU_add_test(pSuite, "test_make_move_castling_rights", test_make_move_castling_rights)) ||
        (NULL == CU_add_test(pSuite, "test_make_move_illegal_movement", test_make_move_illegal_movement)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves", test_legal_moves)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves_pinned_piece", test_legal_moves_pinned_piece)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves_get_out_of_check", test_legal_moves_get_out_of_check)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves_checkmate", test_legal_moves_checkmate)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves_stalemate", test_legal_moves_stalemate)) ||
        (NULL == CU_add_test(pSuite, "test_legal_moves_castling_in_check", test_legal_moves_castling_in_check)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_startpos_moves", test_parse_position_startpos_moves)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_long_move_list", test_parse_position_long_move_list)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_whitespace", test_parse_position_whitespace)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_incomplete_commands", test_parse_position_incomplete_commands)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_malformed_command", test_parse_position_malformed_command)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_invalid_move", test_parse_position_invalid_move)) ||
        (NULL == CU_add_test(pSuite, "test_parse_position_resource_limit", test_parse_position_resource_limit)))
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
