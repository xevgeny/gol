#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <ncurses.h>
#include <string.h>

#define SIZE_COL 10

#define DEAD_CELL '.'
#define LIVE_CELL 'O'

int GRID_H;
int GRID_W;
int GRID_WRAP;

void loadcells(char *fname, char grid[GRID_H][GRID_W]) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int y = 0;

    fp = fopen(fname, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1 && y < GRID_H) {
        if (line[0] == '!')
            continue;
        if (line[0] == '\n') {
            y++;
            continue;
        }
        for (int x = 0; x < read && x < GRID_W; x++)
            if (line[x] == LIVE_CELL)
                grid[y][x] = 1;
        y++;
    }

    fclose(fp);
}

void printgrid(WINDOW *gridw, char grid[GRID_H][GRID_W]) {
    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            if (grid[y][x] == 1)
                mvwaddch(gridw, y, 2*x, LIVE_CELL);
            else
                mvwaddch(gridw, y, 2*x, DEAD_CELL);
        }
    }
}

void evolve(char grid[GRID_H][GRID_W]) {
    char nextgrid[GRID_H][GRID_W];

    for (int y = 0; y < GRID_H; y++) {
        for (int x = 0; x < GRID_W; x++) {
            int n = 0;

            // count neighbors
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0)
                        continue;

                    if (GRID_WRAP)
                        n += grid[(y+i+GRID_H) % GRID_H][(x+j+GRID_W) % GRID_W];
                    else
                        if (y+i >= 0 && y+i < GRID_H && x+j >= 0 && x+j < GRID_W)
                            n += grid[y+i][x+j];
                }
            }

            // evolve cell
            if (grid[y][x] == 0 && n == 3)
                nextgrid[y][x] = 1;
            else if (grid[y][x] != 0 && (n == 2 || n == 3))
                nextgrid[y][x] = 1;
            else
                nextgrid[y][x] = 0;
        }
    }

    for (int y = 0; y < GRID_H; y++) 
        for (int x = 0; x < GRID_W; x++) 
            grid[y][x] = nextgrid[y][x];
}


void printinfo(WINDOW *infow, int it) {
    char s[SIZE_COL];
    snprintf(s, SIZE_COL, "%dx%d", GRID_W, GRID_H);
    mvwaddstr(infow, 0, 0, s);
    snprintf(s, SIZE_COL, "%d", it);
    mvwaddstr(infow, 1, 0, s);

    mvwaddstr(infow, 0, SIZE_COL + 1, "---- ms");
    mvwaddstr(infow, 1, SIZE_COL + 1, "STEP");
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);

    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);

    GRID_H = screen_h - 5;
    GRID_W = screen_w - 2;
    GRID_WRAP = 1;

    char *fname = "./frothingpuffer.cells";

    WINDOW *gridw, *infow;
    refresh(); // draw root window first
    gridw = newwin(GRID_H, 2*GRID_W, 1, 1);
    infow = newwin(2, GRID_W + 2, GRID_H + 2, 1);

    char grid[GRID_H][GRID_W];
    memset(grid, 0, sizeof grid);
    loadcells(fname, grid);

    mvwaddstr(infow, 0, 2*(SIZE_COL + 1), fname);
    if (!GRID_WRAP)
        mvwaddstr(infow, 1, 2*(SIZE_COL + 1), "*");

    int it = 0;
    while (true) {
        printgrid(gridw, grid);
        wrefresh(gridw);

        printinfo(infow, it++);
        wrefresh(infow);

        getch();
        evolve(grid);
    }

    endwin();
    return EXIT_SUCCESS;
}