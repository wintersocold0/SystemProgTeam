
#include "main.h"

int load_current_dir_items(CdFileItem *items, int max_items, int *max_name_len) {
    DIR *dir = opendir(".");
    int n_items = 0;
    *max_name_len = 0;

    if (dir == NULL) {
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && n_items < max_items) {
        snprintf(items[n_items].name, sizeof(items[n_items].name), "%s", entry->d_name);

        struct stat st;
        if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
            items[n_items].is_dir = 1;
        } else {
            items[n_items].is_dir = 0;
        }

        int len = (int)strlen(entry->d_name);
        if (len > *max_name_len) {
            *max_name_len = len;
        }

        n_items++;
    }

    closedir(dir);
    return n_items;
}

void draw_dir_items(WINDOW *win, int win_width, int start_y, int start_x,
                    const CdFileItem *items, int n_items, int max_name_len) {
    int available_width = win_width - 6;

    int col_width = max_name_len + 5;
    if (col_width < 15) col_width = 20;
    if (col_width > 40) col_width = 45;

    int max_cols = available_width / col_width;
    if (max_cols < 1) max_cols = 1;
    if (max_cols > 3) max_cols = 3;

    for (int i = 0; i < n_items && i < 37 ; ++i) {
        int col = i % max_cols;
        int row = i / max_cols;

        int current_y = start_y + row + 2;
        int current_x = start_x + (col * col_width);

        char format_str[32];
        snprintf(format_str, sizeof(format_str), "%%-%ds", col_width - 2);

        mvwprintw(win, current_y, current_x, format_str, items[i].name);
    }
}

// Returns: 1 = Enter pressed, -1 = ESC pressed
int read_line_with_tab(WINDOW *win, int y, int x, int width,
                       char *buf, size_t buf_size,
                       const CdFileItem *items, int n_items) {
    int buf_idx = 0;
    memset(buf, 0, buf_size);

    while (1) {
        mvwprintw(win, y, x, "%-*s", width, "");
        if (buf_idx > width - 1) {
            mvwprintw(win, y, x, "%s", buf + (buf_idx - (width - 1)));
            wmove(win, y, x + width - 1);
        } else {
            mvwprintw(win, y, x, "%s", buf);
            wmove(win, y, x + buf_idx);
        }
        wrefresh(win);

        int ch = wgetch(win);
        if( ch == KEY_RESIZE) {
            return 0; // Handle resize in the calling function
        }else if (ch == '\n' || ch == KEY_ENTER) {
            return 1;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            if (buf_idx > 0) {
                buf_idx--;
                buf[buf_idx] = '\0';
            }
        } else if (ch == '\t') {
            if (items != NULL && n_items > 0) {
                char match[256] = "";
                int match_count = 0;

                for (int i = 0; i < n_items; ++i) {
                    if (strncmp(items[i].name, buf, strlen(buf)) == 0) {
                        if (strcmp(items[i].name, ".") == 0 || strcmp(items[i].name, "..") == 0) {
                            continue;
                        }
                        snprintf(match, sizeof(match), "%s", items[i].name);
                        match_count++;
                    }
                }

                if (match_count > 0) {
                    snprintf(buf, buf_size, "%s", match);
                    buf_idx = (int)strlen(buf);
                }
            }
        } else if (ch >= 32 && ch <= 126 && buf_idx < (int)buf_size - 1) {
            buf[buf_idx++] = (char)ch;
            buf[buf_idx] = '\0';
        } else if (ch == 27) {
            return -1;
        }
    }
}

void run_cd() {
    WINDOW *cd_win=recreate_menu_win(NULL);

    char cwd[1024];
    int win_width,win_height;
    getmaxyx(cd_win, win_height, win_width);
    win_height = win_height;

    while (1) {
        werase(cd_win);
        box(cd_win, 0, 0);
        mvwprintw(cd_win, 0, 4, " Change Directory (CD) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(cd_win, 3, 3, "Current Path: %s", cwd);
        }

        mvwprintw(cd_win, 5, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);

        draw_dir_items(cd_win, win_width, 6, 3, items, n_items, max_name_len);

        mvwprintw(cd_win, 22, 3, "Enter the path name (Press Esc to exit):                        ");
        mvwprintw(cd_win, 23, 3, "Path > ");
        wrefresh(cd_win);

        char input_buf[256] = "";
        int status = read_line_with_tab(cd_win, 23, 10, 50,
                                        input_buf, sizeof(input_buf),
                                        items, n_items);
        if (status == -1) {
            delwin(cd_win);
            return;
        }
        if (status == 0) {
            cd_win =recreate_menu_win(cd_win);
            run_cd();
            return;
            
        }
        if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "EXIT") == 0) {
            break;
        }

        if (chdir(input_buf) == 0) {
            mvwprintw(cd_win, 25, 3, "[Success] The directory has been changed!                     ");
        } else {
            mvwprintw(cd_win, 25, 3, "[Error] Cannot find the folder                                ");
        }
        wrefresh(cd_win);

        wtimeout(cd_win, 1000);
        wgetch(cd_win);
        wtimeout(cd_win, -1);
    }

    delwin(cd_win);
}