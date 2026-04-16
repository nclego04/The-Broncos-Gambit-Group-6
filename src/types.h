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
    char b[64];        /**< Board array: '.' for empty, uppercase for White, lowercase for Black */
    int white_to_move; /**< 1 if it is White's turn to move, 0 if Black's turn */
    int castling;      /**< Bitmask - 1: WK, 2: WQ, 4: BK, 8: BQ (Kingside/Queenside rights) */
    int ep;            /**< En passant target square index (0-63), or -1 if no en passant capture is available */
} Pos;

#endif // TYPES_H