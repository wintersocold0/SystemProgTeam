
#define _POSIX_C_SOURCE 200809L
#include "main.h"
#include <fnmatch.h>

// 1. Search files and directories recursively
static void find_path(const char *path, const char *pattern,
                      int opt_name) {

    struct stat st;

    if (lstat(path, &st) != 0) {
        perror(path);
        return;
    }

    // Extract the name from the path
    char name_buf[4096];

    if (strlen(path) >= sizeof(name_buf)) {
        fprintf(stderr, "[Error] Path is too long.\n");
        return;
    }

    snprintf(name_buf, sizeof(name_buf), "%s", path);

    size_t len = strlen(name_buf);

    while (len > 1 && name_buf[len - 1] == '/') {
        name_buf[--len] = '\0';
    }

    const char *name = strrchr(name_buf, '/');

    if (strcmp(name_buf, "/") == 0)
        name = "/";
    else if (name != NULL)
        name++;
    else
        name = name_buf;

    // Check whether the name matches the pattern
    if (!opt_name || fnmatch(pattern, name, 0) == 0) {
        printf("%s\n", path);
    }

    // Search subdirectories recursively
    if (S_ISDIR(st.st_mode)) {

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

            int n = snprintf(new_path, sizeof(new_path),
                             "%s%s%s", path,
                             path[strlen(path) - 1] == '/' ? "" : "/",
                             entry->d_name);

            if (n < 0 || (size_t)n >= sizeof(new_path)) {
                fprintf(stderr, "[Error] Path is too long.\n");
                continue;
            }

            find_path(new_path, pattern, opt_name);
        }

        closedir(dir);
    }
}


// 2. Main find TUI
void run_find() {

    int highlight = 1;
    int c;

    int opt_name = 1;

    char pattern[256] = "";
    char path[256] = "";
    char cwd[PATH_MAX_LEN];

    WINDOW *find_win = recreate_menu_win(NULL);

    if (find_win == NULL)
        return;

    while (1) {

        int win_h, win_w;
        getmaxyx(find_win, win_h, win_w);

        werase(find_win);

        if (win_h < 13 || win_w < 40) {

            mvwaddnstr(find_win, 0, 0,
                       "Resize terminal or press Esc.",
                       win_w - 1);

            wrefresh(find_win);
            c = wgetch(find_win);

            if (c == 27)
                break;

            if (c == KEY_RESIZE) {

                find_win = recreate_menu_win(find_win);

                if (find_win == NULL)
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
        int name_y    = compact ? 9  : 18;
        int path_y    = compact ? 10 : 21;

        box(find_win, 0, 0);

        mvwprintw(find_win, 0, 4,
                  " find - Search Files ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {

            mvwprintw(find_win, cwd_y, 3,
                      "Current Path: %.*s",
                      win_w - 18, cwd);
        }

        mvwprintw(find_win, guide_y, 3,
                  "%.*s", win_w - 4,
                  "Toggle Options (Press Space to select):");

        // Option -name
        if (highlight == 1)
            wattron(find_win, A_REVERSE);

        mvwprintw(find_win, option_y, 5,
                  "[%s] -name Search by name",
                  opt_name ? "X" : " ");

        if (highlight == 1)
            wattroff(find_win, A_REVERSE);

        // Execute button
        if (highlight == 2)
            wattron(find_win, A_REVERSE);

        mvwprintw(find_win, run_y, 15,
                  " Enter - Run ");

        if (highlight == 2)
            wattroff(find_win, A_REVERSE);

        // Keyboard guide
        if (win_w < 43) {

            mvwprintw(find_win, help_y, 3,
                      "Up/Down | Space | Esc");

        } else {

            mvwprintw(find_win, help_y, 3,
                      "Up/Down Move | Space Toggle | Esc Back");
        }

        wrefresh(find_win);

        c = wgetch(find_win);

        switch (c) {

            case KEY_RESIZE:

                find_win = recreate_menu_win(find_win);

                if (find_win == NULL)
                    return;

                continue;

            case KEY_UP:

                if (highlight > 1)
                    highlight--;

                break;

            case KEY_DOWN:

                if (highlight < 2)
                    highlight++;

                break;

            case ' ':

                if (highlight == 1)
                    opt_name = !opt_name;

                break;

            case 10:
            case KEY_ENTER:

                if (highlight == 2) {

                    int status;

                    // Input name pattern
                    if (opt_name) {

                        if (!compact) {

                            mvwprintw(find_win, name_y - 1, 3,
                                      "Enter name pattern:");
                        }

                        mvwprintw(find_win, name_y, 3,
                                  "Name > ");

                        status = read_line_with_tab(
                            find_win, name_y, 10,
                            win_w - 12,
                            pattern, sizeof(pattern),
                            NULL, 0);

                        if (status == 0) {

                            find_win = recreate_menu_win(find_win);

                            if (find_win == NULL)
                                return;

                            break;
                        }

                        if (status == -1)
                            break;
                    }

                    // Input search path
                    if (!compact) {

                        mvwprintw(find_win, path_y - 1, 3,
                                  "Enter search path:");
                    }

                    mvwprintw(find_win, path_y, 3,
                              "Path > ");

                    CdFileItem items[CD_MAX_ITEMS];
                    int max_name_len = 0;

                    int n_items = load_current_dir_items(
                        items, CD_MAX_ITEMS, &max_name_len);

                    status = read_line_with_tab(
                        find_win, path_y, 10,
                        win_w - 12,
                        path, sizeof(path),
                        items, n_items);

                    if (status == 0) {

                        find_win = recreate_menu_win(find_win);

                        if (find_win == NULL)
                            return;

                        break;
                    }

                    if (status == -1)
                        break;

                    if (strlen(path) == 0)
                        strcpy(path, ".");

                    // Display execution result
                    endwin();

                    printf("\n[실행할 명령어]: find \"%s\"", path);

                    if (opt_name)
                        printf(" -name \"%s\"", pattern);

                    printf("\n");
                    printf("----------------------------------------\n");

                    find_path(path, pattern, opt_name);

                    printf("----------------------------------------\n");
                    printf("계속하려면 Enter를 누르세요...");
                    fflush(stdout);

                    getchar();

                    // Return to ncurses mode
                    refresh();

                    find_win = recreate_menu_win(find_win);

                    if (find_win == NULL)
                        return;
                }

                break;

            case 27:

                delwin(find_win);
                return;
        }
    }

    delwin(find_win);
}

