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

/**
 * Generates all pseudo-legal pawn moves for a given position and square.
 * Handles single pushes, double pushes from the start rank, captures, and promotions.
 */
void gen_pawn(const Pos *p, int from, int white, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    int dir = white ? 1 : -1;    // Pawns move up (+1 rank) for white, down (-1 rank) for black
    int start_r = white ? 1 : 6; // Rank where pawns can double push
    int prom_r = white ? 7 : 0;  // Rank where pawns promote

    int nr = r + dir;
    if (nr >= 0 && nr < 8) {
        // 1. Move forward 1 square
        int to = nr * 8 + f;
        if (p->b[to] == '.') {
            if (nr == prom_r) {
                add_move(moves, n, from, to, 'q');
                add_move(moves, n, from, to, 'r');
                add_move(moves, n, from, to, 'b');
                add_move(moves, n, from, to, 'n');
            } else {
                add_move(moves, n, from, to, 0);
                // 2. Move forward 2 squares (only if forward 1 is also empty and on start rank)
                if (r == start_r) {
                    int to2 = (r + 2 * dir) * 8 + f;
                    if (p->b[to2] == '.') {
                        add_move(moves, n, from, to2, 0);
                    }
                }
            }
        }
        
        // 3. Diagonal captures
        for (int df = -1; df <= 1; df += 2) {
            int nf = f + df;
            if (nf >= 0 && nf < 8) {
                int to_cap = nr * 8 + nf;
                char target = p->b[to_cap];
                // Can capture if square contains an enemy piece
                if (target != '.' && is_white_piece(target) != white) {
                    if (nr == prom_r) {
                        add_move(moves, n, from, to_cap, 'q');
                        add_move(moves, n, from, to_cap, 'r');
                        add_move(moves, n, from, to_cap, 'b');
                        add_move(moves, n, from, to_cap, 'n');
                    } else {
                        add_move(moves, n, from, to_cap, 0);
                    }
                }
            }
        }
    }
}

/**
 * Core move generation logic for sliding pieces (Queen, Rook, Bishop).
 * Follows directional rays until encountering the board edge or another piece.
 */
static void gen_slider(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    for (int i = 0; i < dcount; i++) {
        int nr = r + dirs[i][0], nf = f + dirs[i][1];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int to = nr * 8 + nf;
            char target = p->b[to];
            if (target == '.') {
                add_move(moves, n, from, to, 0);
            } else {
                // Stop sliding on piece collision; add capture if it's an enemy
                if (is_white_piece(target) != white) {
                    add_move(moves, n, from, to, 0);
                }
                break;
            }
            nr += dirs[i][0];
            nf += dirs[i][1];
        }
    }
}

/**
 * Generates all pseudo-legal bishop moves (sliding diagonally).
 */
void gen_bishop(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    for (int i = 0; i < dcount; i++) {
        int nr = r + dirs[i][0], nf = f + dirs[i][1];
        while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int to = nr * 8 + nf;
            char target = p->b[to];
            if (target == '.') {
                add_move(moves, n, from, to, 0);
            } else {
                if (is_white_piece(target) != white) {
                    add_move(moves, n, from, to, 0);
                }
                break;
            }
            nr += dirs[i][0];
            nf += dirs[i][1];
        }
    }
}

/**
 * Generates all pseudo-legal queen moves (sliding in all 8 directions).
 */
void gen_queen(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    gen_slider(p, from, white, dirs, dcount, moves, n);
}
