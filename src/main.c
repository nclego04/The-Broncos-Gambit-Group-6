/**
 * @file main.c
 * @brief Main application entry point and Universal Chess Interface (UCI) command loop.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine.h"
#include "algorithm.h"

int main(void) {
    Pos pos;
    pos_start(&pos);
    
    FILE *f = fopen("tests/search_metrics.txt", "w");
    if (f) fclose(f);

    char line[8192];
    while (fgets(line, sizeof(line), stdin)) {
        // Trim trailing newlines
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
            FILE *f = fopen("tests/search_metrics.txt", "w");
            if (f) fclose(f);
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