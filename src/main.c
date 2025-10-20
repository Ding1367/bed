#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "editor.h"
#include "ui.h"

int main(int argc, char **argv){
    editor_init();
    if (!isatty(STDIN_FILENO)){
        if (argc < 2 && editor_open_file(stdin) == -1){
            perror("failed to read stdin");
            return 1;
        }
        int fd = open("/dev/tty", O_RDONLY, 0);
        if (fd == -1){
            perror("failed to reopen stdin handle");
            return 0;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    } else if (argc > 1){
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL){
            fprintf(stderr, "failed to open file %s: ", argv[1]);
            perror(NULL);
            return 1;
        }
        if (editor_open_file(fp) == -1){
            fprintf(stderr, "failed to read file %s: ", argv[1]);
            perror(NULL);
            return 1;
        }
        editor_set_file_name(argv[1]);
        fclose(fp);
    }
    ui_theme_t theme;
    theme.editor_bg = 17;
    theme.statusline_bg = 20;
    ui_set_theme(&theme);
    ui_init();
    unsigned int line, col, scroll;
    unsigned int num_line;
    int rows, cols;
    while (!editor_should_exit()){
        editor_get_pos(&line, &col);
        scroll = editor_get_scroll();
        num_line = editor_get_line_count();
        ui_get_size(&rows, &cols);
        unsigned int totalLinesUsed = 0;
        unsigned int lineIndex = scroll;
        ui_clear();
        while (totalLinesUsed != (unsigned int)rows - 2) {
            if (lineIndex >= num_line) break;
            int lineLength = 0;
            const char *line = editor_get_line(lineIndex);
            const char *p = line;
            while (*p != '\n')
                lineLength++, p++;
            int used = 1 + lineLength / cols;
            if (totalLinesUsed + used >= (unsigned int)rows - 1)
                used = totalLinesUsed + used - (unsigned int)rows + 2;
            ui_move_cursor(totalLinesUsed, 0);
            ui_print(line, lineLength);
            totalLinesUsed += used;
            lineIndex++;
        }
        int curPhysicalLine = line - scroll + col / cols;
        int curPhysicalCol = col % cols;
        if (curPhysicalLine < 0)
            curPhysicalLine = 0;
        if (curPhysicalLine >= rows - 2)
            curPhysicalLine = rows - 2;
        ui_move_cursor(curPhysicalLine, curPhysicalCol);
        ui_refresh();
        editor_handle_key(getchar());
    }

    ui_end();
    editor_end();
}
