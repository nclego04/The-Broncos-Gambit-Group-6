#include "movegen.h"

/**
 * Generates all pseudo-legal king moves, including castling.
 */
void gen_king(const Pos *p, int from, int white, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int nr = r + dr, nf = f + df;
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                int to = nr * 8 + nf;
                char target = p->b[to];
                // Can move to empty squares or capture enemy pieces
                if (target == '.' || is_white_piece(target) != white) {
                    add_move(moves, n, from, to, 0);
                }
            }
        }
    }

    // Castling logic
    if (white) {
        // White Kingside Castling
        if ((p->castling & 1) && from == 4 && p->b[5] == '.' && p->b[6] == '.' &&
            !is_square_attacked(p, 4, 0) &&
            !is_square_attacked(p, 5, 0) &&
            !is_square_attacked(p, 6, 0)) {
            add_move(moves, n, 4, 6, 0);
        }
        // White Queenside Castling
        if ((p->castling & 2) && from == 4 && p->b[3] == '.' && p->b[2] == '.' && p->b[1] == '.' &&
            !is_square_attacked(p, 4, 0) &&
            !is_square_attacked(p, 3, 0) &&
            !is_square_attacked(p, 2, 0)) {
            add_move(moves, n, 4, 2, 0);
        }
    } else {
        // Black Kingside Castling
        if ((p->castling & 4) && from == 60 && p->b[61] == '.' && p->b[62] == '.' &&
            !is_square_attacked(p, 60, 1) &&
            !is_square_attacked(p, 61, 1) &&
            !is_square_attacked(p, 62, 1)) {
            add_move(moves, n, 60, 62, 0);
        }
        // Black Queenside Castling
        if ((p->castling & 8) && from == 60 && p->b[59] == '.' && p->b[58] == '.' && p->b[57] == '.' &&
            !is_square_attacked(p, 60, 1) &&
            !is_square_attacked(p, 59, 1) &&
            !is_square_attacked(p, 58, 1)) {
            add_move(moves, n, 60, 58, 0);
        }
    }
}