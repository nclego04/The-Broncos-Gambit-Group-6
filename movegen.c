#include "movegen.h"

void gen_pawn(const Pos *p, int from, int white, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    int dir = white ? 1 : -1;
    int start_r = white ? 1 : 6;
    int prom_r = white ? 7 : 0;

    int nr = r + dir;
    if (nr >= 0 && nr < 8) {
        // Move forward 1
        int to = nr * 8 + f;
        if (p->b[to] == '.') {
            if (nr == prom_r) {
                add_move(moves, n, from, to, 'q');
                add_move(moves, n, from, to, 'r');
                add_move(moves, n, from, to, 'b');
                add_move(moves, n, from, to, 'n');
            } else {
                add_move(moves, n, from, to, 0);
                // Move forward 2 (only if forward 1 is also empty and on start rank)
                if (r == start_r) {
                    int to2 = (r + 2 * dir) * 8 + f;
                    if (p->b[to2] == '.') {
                        add_move(moves, n, from, to2, 0);
                    }
                }
            }
        }
        
        // Captures
        for (int df = -1; df <= 1; df += 2) {
            int nf = f + df;
            if (nf >= 0 && nf < 8) {
                int to_cap = nr * 8 + nf;
                char target = p->b[to_cap];
                // Must be an enemy piece
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

void gen_knight(const Pos *p, int from, int white, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    // All 8 possible "L" shape jumps for a knight
    static const int nd[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    
    for (int i = 0; i < 8; i++) {
        int nr = r + nd[i][0], nf = f + nd[i][1];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            int to = nr * 8 + nf;
            char target = p->b[to];
            // Can move if square is empty or has an enemy piece
            if (target == '.' || is_white_piece(target) != white) {
                add_move(moves, n, from, to, 0);
            }
        }
    }

    // Castling
    if (white) {
        if (from == 4 && (p->castling & 1)) { // WK
            if (p->b[5] == '.' && p->b[6] == '.' &&
                !is_square_attacked(p, 4, 0) &&
                !is_square_attacked(p, 5, 0) &&
                !is_square_attacked(p, 6, 0)) {
                add_move(moves, n, 4, 6, 0);
            }
        }
        if (from == 4 && (p->castling & 2)) { // WQ
            if (p->b[3] == '.' && p->b[2] == '.' && p->b[1] == '.' &&
                !is_square_attacked(p, 4, 0) &&
                !is_square_attacked(p, 3, 0) &&
                !is_square_attacked(p, 2, 0)) {
                add_move(moves, n, 4, 2, 0);
            }
        }
    } else {
        if (from == 60 && (p->castling & 4)) { // BK
            if (p->b[61] == '.' && p->b[62] == '.' &&
                !is_square_attacked(p, 60, 1) &&
                !is_square_attacked(p, 61, 1) &&
                !is_square_attacked(p, 62, 1)) {
                add_move(moves, n, 60, 62, 0);
            }
        }
        if (from == 60 && (p->castling & 8)) { // BQ
            if (p->b[59] == '.' && p->b[58] == '.' && p->b[57] == '.' &&
                !is_square_attacked(p, 60, 1) &&
                !is_square_attacked(p, 59, 1) &&
                !is_square_attacked(p, 58, 1)) {
                add_move(moves, n, 60, 58, 0);
            }
        }
    }
}

// Helper function for sliding pieces (Queen, Rook, Bishop)
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
                // If we hit a piece, we can capture if it's an enemy, but we must stop sliding regardless
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

void gen_queen(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    gen_slider(p, from, white, dirs, dcount, moves, n);
}

void gen_bishop(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    gen_slider(p, from, white, dirs, dcount, moves, n);
}

void gen_rook(const Pos *p, int from, int white, const int dirs[][2], int dcount, Move *moves, int *n) {
    gen_slider(p, from, white, dirs, dcount, moves, n);
}

void gen_king(const Pos *p, int from, int white, Move *moves, int *n) {
    int r = from / 8, f = from % 8;
    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int nr = r + dr, nf = f + df;
            if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
                int to = nr * 8 + nf;
                char target = p->b[to];
                if (target == '.' || is_white_piece(target) != white) {
                    add_move(moves, n, from, to, 0);
                }
            }
        }
    }
}