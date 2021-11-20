#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <ncurses.h>
#include <string.h>

#define SIZE_COL 10

#define DEAD_CELL '.'
#define LIVE_CELL 'O'

#define GRID_MODE_AUTO 0
#define GRID_MODE_STEP 1

int GRID_H;
int GRID_W;
int GRID_WRAP;
int GRID_MODE;
int DELAY_MS;

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

    if (GRID_MODE == GRID_MODE_AUTO) {
        snprintf(s, SIZE_COL, "%4d ms", DELAY_MS);
        mvwaddstr(infow, 0, SIZE_COL + 1, s);
        mvwaddstr(infow, 1, SIZE_COL + 1, "AUTO");
    } else {
        mvwaddstr(infow, 0, SIZE_COL + 1, "---- ms");
        mvwaddstr(infow, 1, SIZE_COL + 1, "STEP");
    }
}

void printusage(char *name) {
    fprintf(stderr, "Usage: %s [args] file.cells\n", name);
    fprintf(stderr, "  -n       no wrap\n");
    fprintf(stderr, "  -s       step mode\n");
    fprintf(stderr, "  -d ms    delay in ms\n");
}

int main(int argc, char **argv) {
    GRID_WRAP = TRUE;
    GRID_MODE = GRID_MODE_AUTO;
    DELAY_MS = 50;

    int opt;

    while ((opt = getopt(argc, argv, "nsd:")) != -1) {
        switch (opt) {
        case 'n':
            GRID_WRAP = FALSE;
            break;
        case 's':
            GRID_MODE = GRID_MODE_STEP;
            break;
        case 'd':
            DELAY_MS = atoi(optarg);
            break;
        default:
            printusage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        printusage(argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fname = argv[optind];

    setlocale(LC_ALL, "");
    initscr();
    noecho();
    curs_set(FALSE);

    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);

    GRID_H = screen_h - 5;
    GRID_W = screen_w - 2;

    WINDOW *gridw, *infow;
    refresh(); // draw root window first
    gridw = newwin(GRID_H, 2*GRID_W, 1, 1);
    infow = newwin(2, GRID_W + 2, GRID_H + 2, 1);

    char grid[GRID_H][GRID_W];
    memset(grid, 0, sizeof grid);
    loadcells(fname, grid);

    // print filename and no wrap flag once
    mvwaddstr(infow, 0, 2*(SIZE_COL + 1), fname);
    if (!GRID_WRAP)
        mvwaddstr(infow, 1, 2*(SIZE_COL + 1), "*");

    int it = 0;
    while (true) {
        printgrid(gridw, grid);
        wrefresh(gridw);

        printinfo(infow, it++);
        wrefresh(infow);

        if (GRID_MODE == GRID_MODE_AUTO)
            usleep(DELAY_MS * 1e3);
        else 
            getch();

        evolve(grid);
    }

    endwin();
    return EXIT_SUCCESS;
}