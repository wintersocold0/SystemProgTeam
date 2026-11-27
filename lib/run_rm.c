#include "main.h"


static int confirm_delete(WINDOW *win, int win_w, int prompt_y, const char *target_name) {
    int max_msg = win_w - 6;
    if (max_msg < 16) max_msg = 16;

    char q_buf[560];
    snprintf(q_buf, sizeof(q_buf), "Delete '%s'? [y/N]: ", target_name);
    mvwprintw(win, prompt_y, 3, "%-*.*s", max_msg, max_msg, q_buf);
    wclrtoeol(win);
    box(win, 0, 0);
    wrefresh(win);

    echo();
    int ch = wgetch(win);
    noecho();

    return (ch == 'y' || ch == 'Y');
}

void run_rm() {
    WINDOW *rm_win = recreate_menu_win(NULL);
    char file_input[256] = "";
    char resolved_path[512] = "";
    char cwd[1025] = "";

    int win_width, win_height;
    getmaxyx(rm_win, win_height, win_width);
    while (1) {
        werase(rm_win);
        box(rm_win, 0, 0);
        mvwprintw(rm_win, 0, 4, " Remove File (RM) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(rm_win, 2, 3, "Current Path: %.*s", win_width - 20, cwd);
        }

        mvwprintw(rm_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
        draw_dir_items(rm_win, win_width, 5, 3, items, n_items, max_name_len);

        int guide_y = win_height - 6;
        if (guide_y < 8) guide_y = 8;
        int prompt_y = guide_y + 1;
        int status_y = win_height - 2;
        if (status_y <= prompt_y) status_y = win_height - 1;

        int input_w = win_width - 15;
        if (input_w < 10) input_w = 10;

        mvwprintw(rm_win, guide_y, 3, "Type 'cd <dir>' to move, file name/path to remove, or 'exit'/Esc:");
        mvwprintw(rm_win, prompt_y, 3, "Target > ");
        wrefresh(rm_win);

        int status = read_line_with_tab(rm_win, prompt_y, 12, input_w,
                                        file_input, sizeof(file_input), items, n_items);

        if (status == -2) {
            delwin(rm_win);
            clear();
            refresh();
            run_rm(); // Restart the function to refresh the window and items
            continue;
        }

        if (status == -1) {
            delwin(rm_win);
            return;
        }

        if (strcmp(file_input, "exit") == 0 || strcmp(file_input, "EXIT") == 0) {
            break;
        }

        if (strlen(file_input) == 0) {
            continue;
        }

        if (strncmp(file_input, "cd ", 3) == 0) {
            char target_dir[256];
            snprintf(target_dir, sizeof(target_dir), "%s", file_input + 3);
            if (chdir(target_dir) != 0) {
                char err_buf[320];
                snprintf(err_buf, sizeof(err_buf), "[Error] Cannot cd to '%s' (%s)", target_dir, strerror(errno));
                show_status(rm_win, win_width , status_y, err_buf);
                sleep(1);
            }
            continue;
        }

        build_path(cwd, file_input, resolved_path, sizeof(resolved_path));

        struct stat st;
        if (lstat(resolved_path, &st) != 0) {
            char err_buf[340];
            snprintf(err_buf, sizeof(err_buf), "[Error] Cannot find '%s' (%s)", file_input, strerror(errno));
            show_status(rm_win, win_width, status_y, err_buf);
            sleep(1);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            char err_buf[340];
            snprintf(err_buf, sizeof(err_buf), "[Error] '%s' is a directory (use rmdir)", file_input);
            show_status(rm_win, win_width, status_y, err_buf);
            sleep(1);
            continue;
        }

        if (!confirm_delete(rm_win, win_width, prompt_y, file_input)) {
            show_status(rm_win, win_width, status_y, "[Cancelled] File was not removed.");
            sleep(1);
            continue;
        }

        if (unlink(resolved_path) == 0) {
            char ok_buf[340];
            snprintf(ok_buf, sizeof(ok_buf), "[Success] Removed '%s'!", file_input);
            show_status(rm_win, win_width, status_y, ok_buf);
            sleep(1);
        } else {
            char err_buf[340];
            snprintf(err_buf, sizeof(err_buf), "[Error] Failed to remove '%s' (%s)", file_input, strerror(errno));
            show_status(rm_win, win_width, status_y, err_buf);
            sleep(1);
        }
    }

    delwin(rm_win);
}