
#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include <ctype.h>
#include <sys/stat.h>

// 1. Search for a keyword in a line
static int match_line(const char *line, const char *keyword, int opt_i) {
    if (keyword[0] == '\0')
        return 1;

    if (!opt_i) {
        return strstr(line, keyword) != NULL;
    }

    int len = strlen(keyword);

    for (int i = 0; line[i] != '\0'; i++) {
        int j = 0;

        while (j < len && line[i + j] != '\0' &&
               tolower((unsigned char)line[i + j]) ==
               tolower((unsigned char)keyword[j])) {
            j++;
        }

        if (j == len)
            return 1;
    }

    return 0;
}

// 2. Search inside a file
static void grep_file(const char *path, const char *keyword,
                      int opt_n, int opt_c, int opt_i, int opt_r) {

    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        perror(path);
        return;
    }

    char *line = NULL;
    size_t capacity = 0;
    int line_num = 0;
    int count = 0;

    while (getline(&line, &capacity, fp) != -1) {
        line_num++;

        if (match_line(line, keyword, opt_i)) {
            count++;

            if (!opt_c) {
                if (opt_r)
                    printf("%s:", path);

                if (opt_n)
                    printf("%d:", line_num);

                printf("%s", line);

                if (strchr(line, '\n') == NULL)
                    printf("\n");
            }
        }
    }

    if (opt_c) {
        if (opt_r)
            printf("%s:", path);

        printf("%d\n", count);
    }

    if (ferror(fp))
        perror(path);

    free(line);
    fclose(fp);
}

// 3. Search a file or directory recursively
static void grep_path(const char *path, const char *keyword,
                      int opt_n, int opt_c, int opt_i, int opt_r) {

    struct stat st;

    if (lstat(path, &st) != 0) {
        perror(path);
        return;
    }

    if (S_ISREG(st.st_mode)) {
        grep_file(path, keyword,
                  opt_n, opt_c, opt_i, opt_r);
    }
    else if (S_ISDIR(st.st_mode)) {

        if (!opt_r) {
            printf("[Error] Use -r to search directories.\n");
            return;
        }

        DIR *dir = opendir(path);

        if (dir == NULL) {
            perror(path);
            return;
        }

        struct dirent *entry;
        char new_path[4096];

        while ((entry = readdir(dir)) != NULL) {

            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            int len = snprintf(new_path, sizeof(new_path),
                               "%s/%s", path, entry->d_name);

            if (len < 0 || (size_t)len >= sizeof(new_path)) {
                fprintf(stderr, "[Error] Path is too long.\n");
                continue;
            }

            grep_path(new_path, keyword,
                      opt_n, opt_c, opt_i, opt_r);
        }

        closedir(dir);
    }
}

// 4. Receive text input
static int grep_input(WINDOW *win, int y, int x,
                      char *buf, int size) {

    int idx = 0;
    buf[0] = '\0';

    // Modified: Calculate available input width
    int width = getmaxx(win) - x - 1;

    if (width < 2)
        return -1;

    while (1) {
        mvwprintw(win, y, x, "%-*s", width, "");
        mvwprintw(win, y, x, "%s", buf);
        wmove(win, y, x + idx);
        wrefresh(win);

        int ch = wgetch(win);

        if (ch == KEY_RESIZE) {
            return -2;
        }
        else if (ch == 27) {
            return -1;
        }
        else if (ch == '\n' || ch == KEY_ENTER) {
            return 0;
        }
        else if (ch == KEY_BACKSPACE ||
                 ch == 127 || ch == '\b') {

            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
            }
        }
        else if (ch >= 32 && ch <= 126 &&
                 idx < size - 1 &&
                 idx < width - 1) {

            buf[idx++] = (char)ch;
            buf[idx] = '\0';
        }
    }
}

// 5. Main grep TUI
void run_grep() {

    int highlight = 1;
    int c;

    int opt_n = 0;
    int opt_c = 0;
    int opt_i = 0;
    int opt_r = 0;

    char keyword[256] = "";
    char path[256] = "";
    char cwd[PATH_MAX_LEN];

    // Modified: Use common window creation function
    WINDOW *grep_win = recreate_menu_win(NULL);

    if (grep_win == NULL)
        return;

    while (1) {

        int win_h, win_w;
        getmaxyx(grep_win, win_h, win_w);

        werase(grep_win);

        if (win_h < 13 || win_w < 40) {

            mvwaddnstr(grep_win, 0, 0,
                       "Resize terminal or press Esc.",
                       win_w - 1);

            wrefresh(grep_win);
            c = wgetch(grep_win);

            if (c == 27)
                break;

            if (c == KEY_RESIZE) {
                grep_win = recreate_menu_win(grep_win);

                if (grep_win == NULL)
                    return;
            }

            continue;
        }

        int compact = (win_h < 24);

        int cwd_y     = compact ? 1  : 2;
        int guide_y   = compact ? 2  : 4;
        int option_y  = compact ? 3  : 6;
        int run_y     = compact ? 7  : 12;
        int help_y    = compact ? 8  : 14;
        int keyword_y = compact ? 9  : 18;
        int path_y    = compact ? 10 : 21;

        box(grep_win, 0, 0);

        mvwprintw(grep_win, 0, 4,
                  " grep - Search Text ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {

            // Modified: Limit path output to window width
            mvwprintw(grep_win, cwd_y, 3,
                      "Current Path: %.*s",
                      win_w - 18, cwd);
        }

        mvwprintw(grep_win, guide_y, 3,
                  "%.*s", win_w - 4,
                  "Toggle Options (Press Space to select):");

        // Option -n
        if (highlight == 1)
            wattron(grep_win, A_REVERSE);

        mvwprintw(grep_win, option_y, 5,
                  "[%s] -n Show line numbers",
                  opt_n ? "X" : " ");

        if (highlight == 1)
            wattroff(grep_win, A_REVERSE);

        // Option -c
        if (highlight == 2)
            wattron(grep_win, A_REVERSE);

        mvwprintw(grep_win, option_y + 1, 5,
                  "[%s] -c Count matching lines",
                  opt_c ? "X" : " ");

        if (highlight == 2)
            wattroff(grep_win, A_REVERSE);

        // Option -i
        if (highlight == 3)
            wattron(grep_win, A_REVERSE);

        mvwprintw(grep_win, option_y + 2, 5,
                  "[%s] -i Ignore case",
                  opt_i ? "X" : " ");

        if (highlight == 3)
            wattroff(grep_win, A_REVERSE);

        // Option -r
        if (highlight == 4)
            wattron(grep_win, A_REVERSE);

        mvwprintw(grep_win, option_y + 3, 5,
                  "[%s] -r Recursive search",
                  opt_r ? "X" : " ");

        if (highlight == 4)
            wattroff(grep_win, A_REVERSE);

        // Execute button
        if (highlight == 5)
            wattron(grep_win, A_REVERSE);

        mvwprintw(grep_win, run_y, 15,
                  " Enter - Run ");

        if (highlight == 5)
            wattroff(grep_win, A_REVERSE);

        mvwprintw(grep_win, help_y, 3,
                  "Up/Down Move | Space Toggle | Esc Back");

        // Keep help text inside the window
        if (win_w < 43) {
            mvwprintw(grep_win, help_y, 3,
                      "%-*s", win_w - 4, "");
            mvwprintw(grep_win, help_y, 3,
                      "Up/Down | Space | Esc");
            mvwaddch(grep_win, help_y, win_w - 1, ACS_VLINE);
        }

        wrefresh(grep_win);

        c = wgetch(grep_win);

        switch (c) {

            case KEY_RESIZE:

                grep_win = recreate_menu_win(grep_win);

                if (grep_win == NULL)
                    return;

                continue;

            case KEY_UP:
                if (highlight > 1)
                    highlight--;
                break;

            case KEY_DOWN:
                if (highlight < 5)
                    highlight++;
                break;

            case ' ':
                if (highlight == 1) opt_n = !opt_n;
                else if (highlight == 2) opt_c = !opt_c;
                else if (highlight == 3) opt_i = !opt_i;
                else if (highlight == 4) opt_r = !opt_r;
                break;

            case 10:
            case KEY_ENTER:

                if (highlight == 5) {

                    int result;

                    if (!compact) {
                        mvwprintw(grep_win, keyword_y - 1, 3,
                                  "Enter search keyword:");
                    }

                    mvwprintw(grep_win, keyword_y, 3,
                              "Keyword > ");

                    result = grep_input(grep_win, keyword_y, 13,
                                        keyword, sizeof(keyword));

                    if (result == -2) {
                        grep_win = recreate_menu_win(grep_win);

                        if (grep_win == NULL)
                            return;

                        break;
                    }

                    if (result == -1)
                        break;

                    if (!compact) {
                        mvwprintw(grep_win, path_y - 1, 3,
                                  "Enter file or directory path:");
                    }

                    mvwprintw(grep_win, path_y, 3, "Path > ");

                    result = grep_input(grep_win, path_y, 10,
                                        path, sizeof(path));

                    if (result == -2) {
                        grep_win = recreate_menu_win(grep_win);

                        if (grep_win == NULL)
                            return;

                        break;
                    }

                    if (result == -1)
                        break;

                    if (strlen(path) == 0)
                        strcpy(path, ".");

                    endwin();

                    printf("\n[실행할 명령어]: grep");

                    if (opt_n) printf(" -n");
                    if (opt_c) printf(" -c");
                    if (opt_i) printf(" -i");
                    if (opt_r) printf(" -r");

                    printf(" \"%s\" \"%s\"\n", keyword, path);

                    printf("----------------------------------------\n");

                    grep_path(path, keyword,
                              opt_n, opt_c, opt_i, opt_r);

                    printf("----------------------------------------\n");
                    printf("계속하려면 Enter를 누르세요...");
                    fflush(stdout);
                    getchar();

                    refresh();
                    grep_win = recreate_menu_win(grep_win);

                    if (grep_win == NULL)
                        return;
                }
                break;

            case 27:
                delwin(grep_win);
                return;
        }
    }

    delwin(grep_win);
}
