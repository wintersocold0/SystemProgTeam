#include "main.h"

static int copy_file_fallback(const char *src_path, const char *dest_path) {
    FILE *src = fopen(src_path, "rb");
    if (src == NULL) return -1;

    FILE *dst = fopen(dest_path, "wb");
    if (dst == NULL) {
        fclose(src);
        return -1;
    }

    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            fclose(src);
            fclose(dst);
            unlink(dest_path);
            return -1;
        }
    }

    fclose(src);
    fclose(dst);
    return unlink(src_path);
}

void run_mv() {
    WINDOW *mv_win = recreate_menu_win(NULL);
    char src_input[256] = "";
    char dest_input[256] = "";
    char resolved_src[512] = "";
    char resolved_dest[512] = "";
    char final_dest[1024] = "";
    char cwd[1025] = "";

    int win_width, win_height;
    getmaxyx(mv_win, win_height, win_width);


    while (1) {
        werase(mv_win);
        box(mv_win, 0, 0);
        mvwprintw(mv_win, 0, 4, " Move / Rename File or Folder (MV) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(mv_win, 2, 3, "Current Path: %.*s", win_width - 20, cwd);
        }

        mvwprintw(mv_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
        draw_dir_items(mv_win, win_width, 5, 3, items, n_items, max_name_len);

        int guide_y = win_height - 6;
        if (guide_y < 8) guide_y = 8;
        int prompt_y = guide_y + 1;
        int status_y = win_height - 2;
        if (status_y <= prompt_y) status_y = win_height - 1;

        int input_w = win_width - 15;
        if (input_w < 10) input_w = 10;

        mvwprintw(mv_win, guide_y, 3, "Step 1: 'cd <dir>' to move, source name/path, or 'exit'/Esc:");
        mvwprintw(mv_win, prompt_y, 3, "Source > ");
        wrefresh(mv_win);

        int status = read_line_with_tab(mv_win, prompt_y, 12, input_w,
                                        src_input, sizeof(src_input), items, n_items);

        if (status == -2) {
            delwin(mv_win);
            clear();
            refresh();
            run_mv(); // Restart the function to refresh the window and items
            continue;
        }

        if (status == -1) {
            delwin(mv_win);
            return;
        }

        if (strcmp(src_input, "exit") == 0 || strcmp(src_input, "EXIT") == 0) {
            break;
        }

        if (strlen(src_input) == 0) {
            continue;
        }

        if (strncmp(src_input, "cd ", 3) == 0) {
            char target_dir[256];
            snprintf(target_dir, sizeof(target_dir), "%s", src_input + 3);
            if (chdir(target_dir) != 0) {
                char err_buf[320];
                snprintf(err_buf, sizeof(err_buf), "[Error] Cannot cd to '%s' (%s)", target_dir, strerror(errno));
                show_status(mv_win, win_width , status_y, err_buf);
                sleep(1);
            }
            continue;
        }

        build_path(cwd, src_input, resolved_src, sizeof(resolved_src));

        struct stat src_st;
        if (lstat(resolved_src, &src_st) != 0) {
            char err_buf[320];
            snprintf(err_buf, sizeof(err_buf), "[Error] Source not found: '%s'", src_input);
            show_status(mv_win, win_width , status_y, err_buf);
            sleep(1);
            continue;
        }

        // Step 2: Choose destination directory or new name/path
        int dest_done = 0;
        int cancelled = 0;

        while (!dest_done) {
            werase(mv_win);
            box(mv_win, 0, 0);
            mvwprintw(mv_win, 0, 4, " Move / Rename File or Folder (MV) ");

            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                mvwprintw(mv_win, 2, 3, "Current Path: %.*s", win_width - 20, cwd);
            }

            mvwprintw(mv_win, 3, 3, "Selected: %.*s", win_width - 16, resolved_src);
            mvwprintw(mv_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

            max_name_len = 0;
            n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
            draw_dir_items(mv_win, win_width, 5, 3, items, n_items, max_name_len);

            guide_y = win_height - 6;
            if (guide_y < 8) guide_y = 8;
            prompt_y = guide_y + 1;
            status_y = win_height - 2;
            if (status_y <= prompt_y) status_y = win_height - 1;

            input_w = win_width - 15;
            if (input_w < 10) input_w = 10;

            mvwprintw(mv_win, guide_y, 3, "Step 2: 'cd <dir>' to move, or enter dest folder / new name:");
            mvwprintw(mv_win, prompt_y, 3, "Dest   > ");
            wrefresh(mv_win);

            status = read_line_with_tab(mv_win, prompt_y, 12, input_w,
                                          dest_input, sizeof(dest_input), items, n_items);

            if (status == -2) {
                delwin(mv_win);
                clear();
                refresh();
                run_mv(); // Restart the function to refresh the window and items
                continue;
            }

            if (status == -1) {
                delwin(mv_win);
                return;
            }

            if (strcmp(dest_input, "exit") == 0 || strcmp(dest_input, "EXIT") == 0) {
                cancelled = 1;
                break;
            }

            if (strlen(dest_input) == 0) {
                continue;
            }

            if (strncmp(dest_input, "cd ", 3) == 0) {
                char target_dir[256];
                snprintf(target_dir, sizeof(target_dir), "%s", dest_input + 3);
                if (chdir(target_dir) != 0) {
                    char err_buf[320];
                    snprintf(err_buf, sizeof(err_buf), "[Error] Cannot cd to '%s' (%s)", target_dir, strerror(errno));
                    show_status(mv_win, win_width, status_y, err_buf);
                    sleep(1);
                }
                continue;
            }

            build_path(cwd, dest_input, resolved_dest, sizeof(resolved_dest));

            struct stat dest_st;
            if (stat(resolved_dest, &dest_st) == 0 && S_ISDIR(dest_st.st_mode)) {
                const char *base_name = strrchr(resolved_src, '/');
                base_name = (base_name != NULL) ? base_name + 1 : resolved_src;
                snprintf(final_dest, sizeof(final_dest), "%s/%s", resolved_dest, base_name);
            } else {
                snprintf(final_dest, sizeof(final_dest), "%s", resolved_dest);
            }

            if (rename(resolved_src, final_dest) == 0) {
                char ok_buf[580];
                snprintf(ok_buf, sizeof(ok_buf), "[Success] Moved '%s' -> '%s'", src_input, dest_input);
                show_status(mv_win, win_width, status_y, ok_buf);
                sleep(1);
                dest_done = 1;
            } else if (errno == EXDEV && S_ISREG(src_st.st_mode)) {
                // Cross-filesystem move fallback for regular files
                if (copy_file_fallback(resolved_src, final_dest) == 0) {
                    char ok_buf[580];
                    snprintf(ok_buf, sizeof(ok_buf), "[Success] Moved '%s' -> '%s'", src_input, dest_input);
                    show_status(mv_win, win_width, status_y, ok_buf);
                    sleep(1);
                    dest_done = 1;
                } else {
                    char err_buf[580];
                    snprintf(err_buf, sizeof(err_buf), "[Error] Cross-device move failed (%s)", strerror(errno));
                    show_status(mv_win, win_width, status_y, err_buf);
                    sleep(1);
                }
            } else {
                char err_buf[580];
                snprintf(err_buf, sizeof(err_buf), "[Error] Cannot move (%s)", strerror(errno));
                show_status(mv_win, win_width, status_y, err_buf);
                sleep(1);
            }
        }

        if (cancelled) {
            break;
        }
    }

    delwin(mv_win);
}