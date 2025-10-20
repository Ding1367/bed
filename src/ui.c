#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef struct ui_cell {
    //int fg, bg;
    //char style;
    char ch;
} ui_cell_t;

static struct termios orig;
int rows, cols;
//static int y, x;
static int mv_y, mv_x;
//static int fg_color, bg_color;
//static char style;
static ui_cell_t *front, *back;
ui_theme_t theme;

static void _ui_get_size(void){
    struct winsize sz;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);
    rows = sz.ws_row;
    cols = sz.ws_col;
}

void ui_init(void){
    tcgetattr(STDIN_FILENO, &orig);
    struct termios raw = orig;
    cfmakeraw(&raw);
    printf("\x1b[48;5;%dm\x1b[?1049h\x1b[2J\x1b[H", theme.editor_bg);
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &raw);
    _ui_get_size();
    int area = rows * cols;
    back = calloc(area, sizeof(ui_cell_t));
    front = calloc(area, sizeof(ui_cell_t));
}

void ui_end(void){
    tcsetattr(STDIN_FILENO, TCSADRAIN, &orig);
    printf("\x1b[?1049l\x1b[0m");
    fflush(stdout);
    free(back);
    free(front);
}

void ui_get_size(int *rowsp, int *colsp){
    *rowsp = rows;
    *colsp = cols;
}

void ui_get_theme(ui_theme_t *th){
    *th = theme;
}

void ui_set_theme(const ui_theme_t *th){
    theme = *th;
}

void ui_move_cursor(int y, int x){
    mv_y = y;
    mv_x = x;
}

void ui_print(const char *str, size_t len){
    int p = mv_y * cols + mv_x;
    for (size_t i = 0; i < len; i++){
        back[p].ch = str[i];
        p++;
    }
}

void ui_refresh(void){
    for (int y = 0; y < rows; y++){
        int n = 0;
        printf("\x1b[%dH", y + 1);
        for (int x = 0; x < cols; x++){
            int idx = y * cols + x;
            int ch = back[idx].ch;
            if (front[idx].ch != ch){
                if (ch)
                    printf("\x1b[%dG%c", x + 1, ch);
                front[idx].ch = ch;
                n = ch ? 0 : n + 1;
            }
        }
        if (n)
            printf("\x1b[K");
    }
    printf("\x1b[%d;%dH", mv_y + 1, mv_x + 1);
    fflush(stdout);
}


void ui_clear(void){
    memset(back, 0, sizeof(ui_cell_t) * rows * cols);
}
