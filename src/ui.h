#ifndef __BED_UI_H__
#define __BED_UI_H__
#include <stddef.h>

typedef struct ui_highlight {
    int bg, fg;
    char style;
} ui_highlight_t;

void ui_init(void);
void ui_end(void);
void ui_get_size(int *rows, int *cols);
// returns if the highlight was found
int ui_get_hl(const char *name, ui_highlight_t *th);
// returns -1 on error, 0 if a value was overwritten, and 1 if the highlight is unique
int ui_set_hl(const char *name, const ui_highlight_t *hl);
void ui_move_cursor(unsigned int y, unsigned int x);
void ui_print(const char *str, size_t len);
void ui_refresh(void);
void ui_clear(void);

#endif
