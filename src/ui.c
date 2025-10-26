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

static uint64_t _ui_hash_string(const char *s){
    uint64_t hash = 0xcbf29ce484222325;
    while (*s)
        hash = (hash ^ *s++) * 0x100000001b3;
    return hash;
}

static struct termios orig;
static unsigned int rows, cols;
static struct {
    unsigned int y, x;
} state;
static unsigned int mv_y, mv_x;
//static int fg_color, bg_color;
//static char style;
static ui_cell_t *front, *back;
static ui_theme_t theme;

static void _ui_get_size(void){
    struct winsize sz;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &sz);
    rows = sz.ws_row;
    cols = sz.ws_col;
}

int ui_get_hl(const char *name, ui_highlight_t *th){
}
int ui_set_hl(const char *name, const ui_highlight_t *hl){
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

void ui_move_cursor(unsigned int y, unsigned int x){
    mv_y = y;
    mv_x = x;
}

static void _ui_move_cursor(unsigned int y, unsigned int x){
    if (state.x == x && state.y == y) return;
    int diffX = (int)(state.x - x);
    int diffY = (int)(state.y - y);
    if (diffX && diffY){
        printf("\x1b[%u;%uH", y + 1, x + 1);
    } else if (diffX){
        printf("\x1b[%uG", x + 1);
    } else {
        printf("\x1b[%uH", y + 1);
    }
    state.x = x;
    state.y = y;
}

void ui_print(const char *str, size_t len){
    int p = mv_y * cols + mv_x;
    for (size_t i = 0; i < len; i++){
        back[p].ch = str[i];
        p++;
    }
}

void ui_refresh(void){
    for (unsigned int y = 0; y < rows; y++){
        unsigned int n = 0;
        for (unsigned int x = 0; x < cols; x++){
            int idx = y * cols + x;
            int ch = back[idx].ch;
            if (front[idx].ch != ch){
                if (ch){
                    _ui_move_cursor(y, x);
                    putchar(ch);
                }
                front[idx].ch = ch;
                n = ch ? 0 : n + 1;
            }
        }
        if (n)
            printf("\x1b[K");
    }
    _ui_move_cursor(mv_y, mv_x);
    fflush(stdout);
}


void ui_clear(void){
    memset(back, 0, sizeof(ui_cell_t) * rows * cols);
}
