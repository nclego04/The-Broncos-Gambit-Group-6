#ifndef TYPES_H
#define TYPES_H

/**
 * @brief Represents a single chess move.
 */
typedef struct {
    int from;   /**< 0-63 1D index representing the source square */
    int to;     /**< 0-63 1D index representing the destination square */
    char promo; /**< Piece character to promote to ('q', 'r', 'b', 'n'), or 0 if no promotion */
} Move;

/**
 * @brief Represents the complete state of a chess board.
 */
typedef struct {
    /**
     * @brief Board array representing the 8x8 grid in a 1D layout.
     * '.' for empty, uppercase for White, lowercase for Black.
     * Index 0 is a1, 7 is h1, 56 is a8, and 63 is h8.
     */
    char b[64];
    int white_to_move; /**< 1 if it is White's turn to move, 0 if Black's turn */
    int castling;      /**< Bitmask - 1: WK, 2: WQ, 4: BK, 8: BQ (Kingside/Queenside rights) */
    int ep;            /**< En passant target square index (0-63), or -1 if no en passant capture is available */
} Pos;

#endif // TYPES_H