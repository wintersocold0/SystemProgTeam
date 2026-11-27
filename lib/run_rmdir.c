#include "main.h"


void run_rmdir() {
    WINDOW *rmdir_win=recreate_menu_win(NULL);
    char dir_input[256] = "";
    char resolved_path[512] = "";
    char cwd[1025] = "";

    int win_width,win_height;
    getmaxyx(rmdir_win, win_height, win_width);
    win_height = win_height;

    while (1) {
        werase(rmdir_win);
        box(rmdir_win, 0, 0);
        mvwprintw(rmdir_win, 0, 4, " Remove Empty Directory (RMDIR) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(rmdir_win, 2, 3, "Current Path: %.*s", win_width - 20, cwd);
        }

        mvwprintw(rmdir_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
        draw_dir_items(rmdir_win, win_width, 5, 3, items, n_items, max_name_len);

        int guide_y = win_height - 6;
        if (guide_y < 8) guide_y = 8;
        int prompt_y = guide_y + 1;
        int status_y = win_height - 2;
        if (status_y <= prompt_y) status_y = win_height - 1;

        int input_w = win_width - 15;
        if (input_w < 10) input_w = 10;

        mvwprintw(rmdir_win, guide_y, 3, "Enter empty folder name/path (Tab: complete, 'exit' or Esc to back):");
        mvwprintw(rmdir_win, prompt_y, 3, "Target > ");
        wrefresh(rmdir_win);

        int status = read_line_with_tab(rmdir_win, prompt_y, 12, input_w,
                                        dir_input, sizeof(dir_input), items, n_items);

        if (status == -2) {
            delwin(rmdir_win);
            clear();
            refresh();
            run_rmdir(); // Restart the function to refresh the window and items
            continue;
        }

        if (status == -1) {
            delwin(rmdir_win);
            return;
        }

        if (strcmp(dir_input, "exit") == 0 || strcmp(dir_input, "EXIT") == 0) {
            break;
        }

        if (strlen(dir_input) == 0) {
            continue;
        }

        build_path(cwd, dir_input, resolved_path, sizeof(resolved_path));

        if (rmdir(resolved_path) == 0) {
            char ok_buf[560];
            snprintf(ok_buf, sizeof(ok_buf), "[Success] Removed directory '%s'!", dir_input);
            show_status(rmdir_win, win_width , status_y, ok_buf);
            sleep(1);
        } else {
            char err_buf[580];
            if (errno == ENOTEMPTY || errno == EEXIST) {
                snprintf(err_buf, sizeof(err_buf), "[Error] '%s' is not empty!", dir_input);
            } else if (errno == ENOENT) {
                snprintf(err_buf, sizeof(err_buf), "[Error] Directory '%s' does not exist!", dir_input);
            } else if (errno == ENOTDIR) {
                snprintf(err_buf, sizeof(err_buf), "[Error] '%s' is not a directory!", dir_input);
            } else if (errno == EACCES || errno == EPERM) {
                snprintf(err_buf, sizeof(err_buf), "[Error] Permission denied!");
            } else if (errno == EBUSY) {
                snprintf(err_buf, sizeof(err_buf), "[Error] Directory is currently in use!");
            } else {
                snprintf(err_buf, sizeof(err_buf), "[Error] Failed to remove '%s' (%s)", dir_input, strerror(errno));
            }
            show_status(rmdir_win, win_width , status_y, err_buf);
            sleep(1);
        }
    }

    delwin(rmdir_win);
}