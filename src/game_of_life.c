// Copyright 2022 brietamm,
// coluphi,
// gertudem

#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>


#define ROWS     25
#define COLS     80
#define MAXSPEED 5000
#define MINSPEED 50

#define MIN_TO_LIVE     2
#define MIN_TO_BORN     3
#define MIN_TO_KILL     4

void save_state(int field[][COLS], int save_matr[][COLS]);

int count_live_nieghbours(int field[][COLS], int x, int y);

void new_gen(int field[][COLS], int temp_field[][COLS]);

void draw_world(int world[][COLS], int gen_num,
                unsigned long long dead, int alive);

int init_world(int field[][COLS], int rows, int cols);

int key_event(int *speed, int *pause);

unsigned int get_live_count(int world[][COLS]);

int dead_cells(int cur_world[][COLS], int last_world[][COLS]);

int main() {
    int world[ROWS][COLS];
    int temp_world[ROWS][COLS];
    initscr();
    nodelay(stdscr, 1);
    noecho();
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            temp_world[i][j] = 0;
        }
    }
    int speed = 1000;
    int is_pause = 0;
    if (init_world(world, ROWS, COLS) != -1) {
        int Qflag = 0;
        if (freopen("/dev/tty", "r", stdin) == NULL) {
            printw("n/a");
            refresh();
            Qflag = 1;
        }
        int living = get_live_count(world);
        unsigned int cur_gen = 0;
        unsigned long long dead = 0;
        if (!Qflag) {
            draw_world(world, cur_gen, dead, living);
            save_state(world, temp_world);
            new_gen(world, temp_world);
            Qflag = key_event(&speed, &is_pause);
            usleep(speed * 1000);
        }
        while (!Qflag && living != 0) {
            ++cur_gen;
            living = get_live_count(world);
            dead += living - dead_cells(world, temp_world);
            draw_world(world, cur_gen, dead, living);
            save_state(world, temp_world);
            new_gen(world, temp_world);
            Qflag = key_event(&speed, &is_pause);
        }
    } else {
        printw("n/a");
        refresh();
    }
    endwin();
    return 0;
}

int key_event(int *speed, int *pause) {
    int quit_flag = 0;
    char ch = getch();
    timeout(*speed);
    if (ch == 'z' || ch == 'Z') {
        if (*speed < MAXSPEED)
            (*speed) += 50;
    }
    if (ch == 'x' || ch == 'X') {
        if (*speed > MINSPEED + 50)
            (*speed) -= 50;
    }
    if (ch == 'q' || ch == 'Q')
        quit_flag = 1;
    if (ch == 'p' || ch == 'P') {
        if (*pause == 0) {
            nodelay(stdscr, 1);
            *pause = 1;
            getch();
        } else {
            nodelay(stdscr, 0);
            *pause = 0;
            getch();
        }
    }
    return quit_flag;
}

int count_live_nieghbours(int field[][COLS], int row, int col) {
    int count = 0;
    for (int i = row - 1; i < row + 2; ++i) {
        for (int j = col - 1; j < col + 2; ++j) {
            int curi = i, curj = j;
            if (i == -1)
                curi = ROWS - 1;
            if (i == ROWS)
                curi = 0;
            if (j == -1)
                curj = COLS - 1;
            if (j == COLS)
                curj = 0;
            if ((curi != row || curj != col) && field[curi][curj] == 1)
                ++count;
        }
    }
    return count;
}

void save_state(int field[][COLS], int save_matr[][COLS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            save_matr[i][j] = field[i][j];
        }
    }
}

void new_gen(int field[][COLS], int temp_field[][COLS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            int neighbours = count_live_nieghbours(temp_field, i, j);
            if (neighbours < MIN_TO_LIVE || neighbours >= MIN_TO_KILL)
                field[i][j] = 0;
            else if (temp_field[i][j] == 0 && neighbours == MIN_TO_BORN)
                field[i][j] = 1;
        }
    }
}

void draw_world(int world[][COLS], int gen_num,
                unsigned long long dead, int alive) {
    clear();
    printw("Living cells: %d\n", alive);
    printw("Current generation number: %d\n", gen_num);
    printw("Total dead cells: %d\n", dead);
    printw(" ");
    for (int j = 0; j < COLS; ++j)
        printw("==");
    printw("\n");
    for (int i = 0; i < ROWS; ++i) {
        printw("|");
        for (int j = 0; j < COLS; ++j) {
            if (world[i][j] == 1)
                printw("*");
            else
                printw(" ");
            printw(" ");
        }
        printw("|");
        printw("\n");
    }
    printw(" ");
    for (int j = 0; j < COLS; ++j)
        printw("==");
    printw("\nZ - decrease the game speed;\n");
    printw("X - increase the game speed;\n");
    printw("P - pause;\n");
    printw("Q - close the game\n");
    refresh();
}

int init_world(int field[][COLS], int rows, int cols) {
    int x, y;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            field[i][j] = 0;
    int flag = scanf("%d, %d", &x, &y);
    if (flag != 2)
        flag = -1;
    while (flag == 2 && x != -1 && y != -1) {
        field[y][x] = 1;
        scanf("%d, %d", &x, &y);
    }
    return flag;
}

int dead_cells(int cur_world[][COLS], int last_world[][COLS]) {
    int counter = 0;
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (cur_world[i][j] != last_world[i][j] && cur_world[i][j] == 1) {
                ++counter;
            }
        }
    }
    return counter;
}

unsigned int get_live_count(int world[][COLS]) {
    unsigned int count = 0;
    unsigned i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (world[i][j] == 1) {
                count++;
            }
        }
    }
    return count;
}

// TODO(brietamm): to fix problems with conrol
// TODO(brietamm): to fix input errors
// TODO(brietamm): maybe to fix statistic
