#ifndef __BED_UI_H__
#define __BED_UI_H__
#include <stddef.h>

typedef struct ui_theme {
    int editor_bg;
    int statusline_bg;
} ui_theme_t;

void ui_init(void);
void ui_end(void);
void ui_get_size(int *rows, int *cols);
void ui_get_theme(ui_theme_t *th);
void ui_set_theme(const ui_theme_t *th);
void ui_move_cursor(int y, int x);
void ui_print(const char *str, size_t len);
void ui_refresh(void);
void ui_clear(void);

#endif
