#include "editor.h"
#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

typedef struct buffer_line {
    char *line;
    size_t capacity;
} buffer_line_t;

#define NO_CMB  0
#define CMB_NUM 1
#define CMB_ACTION 2
#define CMB_TARGET 3

#define TARGET_TYPE_AROUND  0
#define TARGET_TYPE_INSIDE  1
#define TARGET_TYPE_DEFAULT 2

static buffer_line_t *buffer;
static char *filename;
static unsigned int num_line;
static unsigned int cap_line;
static struct {
    unsigned int line, col, abs_col;
    unsigned int scroll;
} pos;
static int mode;
static char quit;
static struct {
    char state;
    unsigned int n;
    char action;
    struct {
        char kind;
        char subtarget;
    } target;
} combo;
//static struct {
//    unsigned int start;
//    unsigned int end;
//    unsigned int length;
//    char gap[256];
//} gap;

void editor_get_pos(unsigned int *line, unsigned int *col){
    *line = pos.line;
    *col = pos.col;
}
unsigned int editor_get_scroll(void){
    return pos.scroll;
}
int editor_get_mode(void){
    return mode;
}
unsigned int editor_get_line_count(void){
    return num_line;
}
int editor_should_exit(void){
    return quit;
}
const char *editor_get_line(unsigned int num){
    if (num >= num_line)
        return NULL;
    return buffer[num].line;
}

#define EDITOR_INIT_LINE_CAP 8

static void _editor_free_buf(void){
    for (unsigned int i = 0; i < cap_line; i++){
        free(buffer[i].line);
    }
    free(buffer);
    num_line = cap_line = 0;
}

int editor_empty_buffer(void){
    buffer_line_t *newBuffer = calloc(EDITOR_INIT_LINE_CAP, sizeof(buffer_line_t));
    char *emptyLine = strdup("\n");
    if (!newBuffer || !emptyLine){
        free(newBuffer);
        free(emptyLine);
        return 0;
    }
    _editor_free_buf();
    num_line = 1;
    cap_line = EDITOR_INIT_LINE_CAP;
    buffer = newBuffer;
    buffer[0].line = emptyLine;
    buffer[0].capacity = 2;
    return 1;
}

int editor_init(void){
    return editor_empty_buffer();
}

int editor_open_file(FILE *fp){
    pos.line = 0;
    pos.col = 0;
    pos.scroll = 0;
    for (unsigned int i = 0; i < cap_line; i++){
        free(buffer[i].line);
    }
    free(buffer);
    buffer = calloc(cap_line = EDITOR_INIT_LINE_CAP, sizeof(buffer_line_t));
    errno = 0;
    num_line = 0;
    size_t cap = 0;
    size_t len;
    char *line = NULL;
    while ((len = getline(&line, &cap, fp)) != (size_t)-1){
        if (num_line + 1 == cap_line){
            // prepare the next iteration
            buffer = realloc(buffer, sizeof(buffer_line_t) * (cap_line <<= 1));
        }
        buffer_line_t *buf_line = &buffer[num_line];
        buf_line->line = line;
        buf_line->capacity = cap;
        num_line++;
        line = NULL;
        cap = 0;
    }
    if (errno == 0){
        if (num_line == 0)
            editor_empty_buffer();
        else
            buffer[num_line - 1].line[len] = 0;
        return 0;
    }
    return -1;
}

int editor_set_file_name(const char *name){
    filename = strdup(name);
    return filename != NULL;
}

void editor_end(void){
    _editor_free_buf();
}

static void _editor_move_line(unsigned int lineNumber){
    pos.col = pos.abs_col;
    pos.line = lineNumber;
    size_t len = editor_line_length(buffer[lineNumber].line);
    if (pos.col > len){
        pos.col = len;
    }
}

static void _editor_combo_tick(int ch){
    int rows, cols;
    ui_get_size(&rows, &cols);
    char cur_state = combo.state;
    if (cur_state == NO_CMB || cur_state == CMB_NUM){
        if (isdigit(ch)){
            if (cur_state == NO_CMB){
                if (ch == '0') return;
                combo.n = 0;
            }
            combo.n = combo.n * 10 + ch - '0';
            combo.state = CMB_NUM;
        } else {
            combo.action = ch;
            if (cur_state == NO_CMB)
                combo.n = 1;
            combo.state = CMB_ACTION;
            static const char *one_key_actions = "hjkliag";
            if (strchr(one_key_actions, ch) && ch){
                goto exec;
            }
        }
    } else if (cur_state == CMB_ACTION){
        combo.state = CMB_TARGET;
        if (ch == combo.action){
            combo.target.kind = TARGET_TYPE_DEFAULT;
            goto exec;
        } else if (ch == 'i'){
            combo.target.kind = TARGET_TYPE_INSIDE;
        } else if (ch == 'a'){
            combo.target.kind = TARGET_TYPE_AROUND;
        } else {
            combo.state = NO_CMB;
        }
    } else if (cur_state == CMB_TARGET){
        combo.target.subtarget = ch;
        goto exec;
    }
    return;
exec:
    combo.state = NO_CMB;
    int act = combo.action;
    unsigned int n = combo.n;
    switch (act){
    case 'j':
        if (pos.line == num_line - 1) break;
        if (pos.line + n >= num_line) {
            _editor_move_line(num_line - 1);
            pos.scroll = num_line - 1;
            break;
        }
        _editor_move_line(pos.line + n);
        while (pos.line - pos.scroll >= (unsigned int)rows - 2){
            pos.scroll++;
        }
        break;
    case 'k':
        if (pos.line < n) {
            _editor_move_line(0);
            pos.scroll = 0;
            break;
        }
        _editor_move_line(pos.line - n);
        while (pos.line - pos.scroll < 0){
            pos.scroll--;
        }
        break;
    case 'h':
        if (pos.col == 0) break;
        if (pos.col < n) {
            pos.col = pos.abs_col = 0;
            break;
        }
        pos.abs_col = pos.col -= n;
        break;
    case 'l': {
        char *line = buffer[pos.line].line;
        size_t lineLength = editor_line_length(line);
        if (pos.col > lineLength) break;
        if (lineLength < pos.col + n)
            pos.col = lineLength;
        else
            pos.col += n;
        pos.abs_col = pos.col;
    } break;
    case 'g': {
        unsigned int line = n - 1;
        if (line >= num_line)
            line = num_line - 1;
        size_t lineLength = editor_line_length(buffer[line].line);
        pos.line = line;
        pos.scroll = line - rows / 2;
        if (pos.col > lineLength)
            pos.col = lineLength;
        if (pos.scroll < 0)
            pos.scroll = 0;
    } break;
    default: break;
    }
    //for (unsigned int i = 0; i < combo.n; ++i){
    //}
}

static void _editor_normal_key(int ch){
    if(ch==17)
        quit = 1;
    else
        _editor_combo_tick(ch);
}

void editor_handle_key(int ch){
    switch(mode){
    case EDITOR_NORMAL:
        _editor_normal_key(ch);
        break;
    default: break;
    }
}

size_t editor_line_length(const char *line){
    size_t c = 0;
    while (line[c] != '\n')
        c++;
    return c;
}
