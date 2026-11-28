#include "main.h"
#include <fcntl.h>

void run_cat() {
    WINDOW *cat_win = recreate_menu_win(NULL);

    char cwd[1024];
    int win_width, win_height;

    getmaxyx(cat_win, win_height, win_width);

    while (1) {
        werase(cat_win);
        box(cat_win, 0, 0);
        mvwprintw(cat_win, 0, 4, " File Content (CAT) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(cat_win, 3, 3, "Current Path: %s", cwd);
        }

        mvwprintw(cat_win, 5, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;

        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);

        draw_dir_items(cat_win, win_width, 6, 3, items, n_items, max_name_len);

        mvwprintw(cat_win, 22, 3, "Enter the file name (Press Esc to exit):");
        mvwprintw(cat_win, 23, 3, "File > ");
        wrefresh(cat_win);

        char filename[256] = "";

        int status = read_line_with_tab(
            cat_win, 23, 10, 50,
            filename, sizeof(filename),
            items, n_items
        );

        if (status == -1) {
            break;
        }

        if (status == 0) {
            cat_win = recreate_menu_win(cat_win);
            getmaxyx(cat_win, win_height, win_width);
            continue;
        }

        if (strcmp(filename, "exit") == 0) {
            break;
        }

        if (filename[0] == '\0') {
            continue;
        }

        int fd = open(filename, O_RDONLY);

        if (fd == -1) {
            mvwprintw(cat_win, 25, 3, "Cannot open file");
            wrefresh(cat_win);
            wgetch(cat_win);
            continue;
        }

        werase(cat_win);
        box(cat_win, 0, 0);

        mvwprintw(cat_win, 0, 4, " File Content (CAT) ");
        mvwprintw(cat_win, 2, 3, "File: %s", filename);

        char buffer[1024];
        ssize_t n;
        int row = 4;
        int col = 3;

        scrollok(cat_win, TRUE);

        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
            for (int i = 0; i < n; i++) {
                if (buffer[i] == '\n') {
                    row++;
                    col = 3;
                } else {
                    mvwaddch(cat_win, row, col, buffer[i]);
                    col++;

                    if (col >= win_width - 3) {
                        row++;
                        col = 3;
                    }
                }

                if (row >= win_height - 2) {
                    wscrl(cat_win, 1);
                    row--;
                }
            }
        }

        close(fd);

        scrollok(cat_win, FALSE);

        mvwprintw(cat_win, win_height - 2, 3, "End of file. Press any key.");

        wrefresh(cat_win);
        wgetch(cat_win);
    }

    delwin(cat_win);
}