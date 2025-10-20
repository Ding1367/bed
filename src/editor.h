#ifndef __BED_EDITOR_H__
#define __BED_EDITOR_H__
#include <stdio.h>

#define EDITOR_NORMAL  0
#define EDITOR_INSERT  1
#define EDITOR_COMMAND 2

void editor_get_pos(unsigned int *line, unsigned int *col);
unsigned int editor_get_scroll(void);
int editor_get_mode(void);
unsigned int editor_get_line_count(void);
int editor_should_exit(void);
const char *editor_get_line(unsigned int num);
void editor_handle_key(int ch);
int editor_empty_buffer(void);
int editor_init(void);
int editor_open_file(FILE *fp);
int editor_set_file_name(const char *name);
void editor_end(void);
size_t editor_line_length(const char *line);

#endif
