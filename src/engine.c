/**
 * @file engine.c
 * @brief Main Universal Chess Interface (UCI) loop and core board state management.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "movegen.h"

/**
 * @brief Converts a standard algebraic notation square (e.g., "e2") to a 0-63 board index.
 */
static int sq_index(const char *s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

/**
 * @brief Converts a 0-63 board index back to standard algebraic notation.
 */
void index_to_sq(int idx, char out[3]) {
    out[0] = (char) ('a' + (idx % 8));
    out[1] = (char) ('1' + (idx / 8));
    out[2] = 0;
}

/**
 * @brief Initializes a position state struct from a standard FEN string.
 * @param p The position pointer to initialize.
 * @param fen The Forsyth-Edwards Notation string.
 */
void pos_from_fen(Pos *p, const char *fen) {
    memset(p->b, '.', 64);
    p->white_to_move = 1;
    p->castling = 0;
    p->ep = -1;

    char buf[256];
    strncpy(buf, fen, sizeof(buf)-1);
    buf[sizeof(buf) - 1] = 0;

    char *save = NULL;
    char *placement = strtok_r(buf, " ", &save);
    char *stm = strtok_r(NULL, " ", &save);
    if (stm) p->white_to_move = (strcmp(stm, "w") == 0);

    // Parse castling rights into the bitmask representation
    char *castle = strtok_r(NULL, " ", &save);
    if (castle && strcmp(castle, "-") != 0) {
        if (strchr(castle, 'K')) p->castling |= 1;
        if (strchr(castle, 'Q')) p->castling |= 2;
        if (strchr(castle, 'k')) p->castling |= 4;
        if (strchr(castle, 'q')) p->castling |= 8;
    }

    // Parse the En Passant target square
    char *ep = strtok_r(NULL, " ", &save);
    if (ep && strcmp(ep, "-") != 0) {
        p->ep = sq_index(ep);
    }

    int rank = 7, file = 0;
    // Parse piece placement data
    for (size_t i = 0; placement && placement[i]; i++) {
        char c = placement[i];
        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }
        if (isdigit((unsigned char) c)) {
            file += c - '0';
            continue;
        }
        int idx = rank * 8 + file;
        if (idx >= 0 && idx < 64) p->b[idx] = c;
        file++;
    }
}

static void pos_start(Pos *p) {
    pos_from_fen(p, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

int is_white_piece(char c) { return c >= 'A' && c <= 'Z'; }

/**
 * @brief Checks if a given square is attacked by a specific color.
 * @param p The board position.
 * @param sq The index of the square to check.
 * @param by_white 1 to check for white attackers, 0 for black attackers.
 */
int is_square_attacked(const Pos *p, int sq, int by_white) {
    int r = sq / 8, f = sq % 8;

    // pawns
    if (by_white) {
        if (r > 0 && f > 0 && p->b[(r - 1) * 8 + (f - 1)] == 'P') return 1;
        if (r > 0 && f < 7 && p->b[(r - 1) * 8 + (f + 1)] == 'P') return 1;
    } else {
        if (r < 7 && f > 0 && p->b[(r + 1) * 8 + (f - 1)] == 'p') return 1;
        if (r < 7 && f < 7 && p->b[(r + 1) * 8 + (f + 1)] == 'p') return 1;
    }

    // knights
    static const int nd[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    for (int i = 0; i < 8; i++) {
        int to = sq + nd[i];
        if (to < 0 || to >= 64) continue;
        int tr = to / 8, tf = to % 8;
        int dr = tr - r;
        if (dr < 0) dr = -dr;
        int df = tf - f;
        if (df < 0) df = -df;
        if (!((dr == 1 && df == 2) || (dr == 2 && df == 1))) continue;
        char pc = p->b[to];
        if (by_white && pc == 'N') return 1;
        if (!by_white && pc == 'n') return 1;
    }

    // sliders
    static const int dirs[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    for (int di = 0; di < 8; di++) {
        int df = dirs[di][0], dr = dirs[di][1];
        int cr = r + dr, cf = f + df;
        while (cr >= 0 && cr < 8 && cf >= 0 && cf < 8) {
            int idx = cr * 8 + cf;
            char pc = p->b[idx];
            if (pc != '.') { // Collision detected
                int pc_white = is_white_piece(pc);
                if (pc_white == by_white) {
                    char up = (char) toupper((unsigned char) pc);
                    int rook_dir = (di < 4);
                    int bishop_dir = (di >= 4);
                    if (up == 'Q') return 1;
                    if (rook_dir && up == 'R') return 1;
                    if (bishop_dir && up == 'B') return 1;
                    if (up == 'K' && (abs(cr - r) <= 1 && abs(cf - f) <= 1)) return 1;
                }
                break;
            }
            cr += dr;
            cf += df;
        }
    }

    // king adjacency (extra safety)
    for (int rr = r - 1; rr <= r + 1; rr++) {
        for (int ff = f - 1; ff <= f + 1; ff++) {
            if (rr < 0 || rr >= 8 || ff < 0 || ff >= 8) continue;
            if (rr == r && ff == f) continue;
            char pc = p->b[rr * 8 + ff];
            if (by_white && pc == 'K') return 1;
            if (!by_white && pc == 'k') return 1;
        }
    }

    return 0;
}

/**
 * @brief Helper function to quickly ascertain if the specified side's king is in check.
 */
static int in_check(const Pos *p, int white_king) {
    char k = white_king ? 'K' : 'k';
    int ksq = -1;
    for (int i = 0; i < 64; i++) if (p->b[i] == k) {
        ksq = i;
        break;
    }
    if (ksq < 0) return 1;
    return is_square_attacked(p, ksq, !white_king);
}

/**
 * @brief Executes a move on the board and produces a new position state.
 * Handles special moves like castling, en passant, and promotions automatically.
 * @return A new Pos struct representing the board after the move.
 */
Pos make_move(const Pos *p, Move m) {
    Pos np = *p;
    char piece = np.b[m.from];
    np.b[m.from] = '.';
    char placed = piece;
    if (m.promo && (piece == 'P' || piece == 'p')) {
        placed = is_white_piece(piece)
                     ? (char) toupper((unsigned char) m.promo)
                     : (char) tolower((unsigned char) m.promo);
    }
    np.b[m.to] = placed;

    // Castling processing
    if ((piece == 'K' || piece == 'k') && abs(m.to - m.from) == 2) {
        if (m.to == 6) { np.b[5] = 'R'; np.b[7] = '.'; }         // White Kingside
        else if (m.to == 2) { np.b[3] = 'R'; np.b[0] = '.'; }    // White Queenside
        else if (m.to == 62) { np.b[61] = 'r'; np.b[63] = '.'; } // Black Kingside
        else if (m.to == 58) { np.b[59] = 'r'; np.b[56] = '.'; } // Black Queenside
    }

    np.ep = -1;
    
    // Update castling rights
    // Disable rights if a king moves, or if rooks move/are captured
    if (piece == 'K') np.castling &= ~3;   // Both White rights
    else if (piece == 'k') np.castling &= ~12; // Both Black rights
    if (m.from == 7 || m.to == 7) np.castling &= ~1;   // WK rook
    if (m.from == 0 || m.to == 0) np.castling &= ~2;   // WQ rook
    if (m.from == 63 || m.to == 63) np.castling &= ~4; // BK rook
    if (m.from == 56 || m.to == 56) np.castling &= ~8; // BQ rook

    np.white_to_move = !p->white_to_move;
    return np;
}

/**
 * @brief Helper utility to safely add a formatted move to a target move list.
 */
void add_move(Move *moves, int *n, int from, int to, char promo) {
    moves[*n].from = from;
    moves[*n].to = to;
    moves[*n].promo = promo;
    (*n)++;
}

/**
 * @brief Populates the `moves` array with all pseudo-legal moves for the active side.
 * Pseudo-legal moves adhere to piece movement geometry but don't consider if the king is left in check.
 * @return The number of pseudo-legal moves generated.
 */
static int pseudo_legal_moves(const Pos *p, Move *moves) {
    int n = 0;
    int us_white = p->white_to_move;
    for (int i = 0; i < 64; i++) {
        char pc = p->b[i];
        if (pc == '.') continue;
        int white = is_white_piece(pc);
        if (white != us_white) continue;
        char up = (char) toupper((unsigned char) pc);
        if (up == 'P') gen_pawn(p, i, white, moves, &n);
        else if (up == 'N') gen_knight(p, i, white, moves, &n);
        else if (up == 'B') {
            static const int d[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
            gen_bishop(p, i, white, d, 4, moves, &n);
        } else if (up == 'R') {
            static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            gen_rook(p, i, white, d, 4, moves, &n);
        } else if (up == 'Q') {
            static const int d[8][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            gen_queen(p, i, white, d, 8, moves, &n);
        } else if (up == 'K') gen_king(p, i, white, moves, &n);
    }
    return n;
}

/**
 * @brief Generates strictly legal moves by validating pseudo-legal ones against king safety.
 * @param p The board position state.
 * @param out Array populated with valid, legal moves.
 * @return Number of legal moves stored in `out`.
 */
int legal_moves(const Pos *p, Move *out) {
    Move tmp[256];
    int pn = pseudo_legal_moves(p, tmp);
    int n = 0;
    for (int i = 0; i < pn; i++) {
        Pos np = make_move(p, tmp[i]);
        // After the move, we must ensure the side who just moved isn't in check
        if (!in_check(&np, !np.white_to_move)) {
            out[n++] = tmp[i];
        }
    }
    return n;
}

/**
 * @brief Parses a UCI move string (e.g., "e2e4" or "e7e8q") and applies it to the position.
 * @param uci The standard coordinate notation string.
 */
static void apply_uci_move(Pos *p, const char *uci) {
    if (!uci || strlen(uci) < 4) return;
    Move m;
    m.from = sq_index(uci);
    m.to = sq_index(uci + 2);
    m.promo = (strlen(uci) >= 5) ? uci[4] : 0;
    Pos np = make_move(p, m);
    *p = np;
}

/**
 * @brief Parses the UCI "position" command string, updating the engine's internal board state.
 * Handles both "startpos" initialization and raw FEN string setups, followed by a move list.
 * @param line The full command string from standard input.
 */
static void parse_position(Pos *p, const char *line) {
    // position startpos [moves ...]
    // position fen <6 fields> [moves ...]
    char buf[8192];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf) - 1] = 0;

    char *toks[2048];
    int nt = 0;
    char *save = NULL;
    for (char *tok = strtok_r(buf, " \t\r\n", &save); tok && nt < 2048; tok = strtok_r(NULL, " \t\r\n", &save)) {
        toks[nt++] = tok;
    }

    int i = 1;
    if (i < nt && strcmp(toks[i], "startpos") == 0) {
        pos_start(p);
        i++;
    } else if (i < nt && strcmp(toks[i], "fen") == 0) {
        i++;
        char fen[512] = {0};
        for (int k = 0; k < 6 && i < nt; k++, i++) {
            if (k)
                strcat(fen, " ");
            strcat(fen, toks[i]);
        }
        pos_from_fen(p, fen);
    }

    if (i < nt && strcmp(toks[i], "moves") == 0) {
        i++;
        for (; i < nt; i++) apply_uci_move(p, toks[i]);
    }
}

void print_bestmove(Move m) {
    char a[3], b[3];
    index_to_sq(m.from, a);
    index_to_sq(m.to, b);
    if (m.promo) printf("bestmove %s%s%c\n", a, b, m.promo);
    else printf("bestmove %s%s\n", a, b);
    fflush(stdout);
}

int main(void) {
    Pos pos;
    pos_start(&pos);

    char line[8192];
    while (fgets(line, sizeof(line), stdin)) {
        // trim
        size_t len = strlen(line);
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = 0;
        if (!len) continue;

        if (strcmp(line, "uci") == 0) {
            printf("id name team6\n");
            printf("id author group6\n");
            printf("uciok\n");
            fflush(stdout);
        } else if (strcmp(line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);
        } else if (strcmp(line, "ucinewgame") == 0) {
            pos_start(&pos);
        } else if (strncmp(line, "position", 8) == 0) {
            parse_position(&pos, line);
        } else if (strncmp(line, "go", 2) == 0) {
            search_position(&pos, line);
        } else if (strcmp(line, "quit") == 0) {
            break;
        }
    }
    return 0;
}
