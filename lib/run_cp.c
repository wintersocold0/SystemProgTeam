#include "main.h"

void run_cp() {
    WINDOW *cp_win=recreate_menu_win(NULL);
    char src_input[256] = "";
    char dest_input[256] = "";
    char resolved_src[512] = "";
    char resolved_dest[512] = "";
    char cwd[1025] = "";

    int win_width,win_height;
    getmaxyx(cp_win, win_height, win_width);
    win_height = win_height;
    while (1) {
        werase(cp_win);
        box(cp_win, 0, 0);
        mvwprintw(cp_win, 0, 4, " Copy File (CP) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(cp_win, 2, 3, "Current Path: %s", cwd);
        }

        mvwprintw(cp_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
        draw_dir_items(cp_win, win_width, 6, 3, items, n_items, max_name_len);

        mvwprintw(cp_win, 22, 3, "Step 1: Type 'cd <dir>' to move, file name/path to copy, or 'exit' / Esc:");
        mvwprintw(cp_win, 23, 3, "Source > ");
        wrefresh(cp_win);

        int status = read_line_with_tab(cp_win, 23, 12, 75, src_input, sizeof(src_input), items, n_items);
        if(status == 0){
            delwin(cp_win);
            run_cp(); // Restart the function to refresh the window and items
            return;
        }
        if (status == -1) {
            delwin(cp_win);
            return;
        }

        if (strcmp(src_input, "exit") == 0 || strcmp(src_input, "EXIT") == 0) {
            break;
        }

        if (strlen(src_input) == 0) {
            continue;
        }

        // Navigate directories while selecting source
        if (strncmp(src_input, "cd ", 3) == 0) {
            char target_dir[256];
            snprintf(target_dir, sizeof(target_dir), "%s", src_input + 3);
            if (chdir(target_dir) != 0) {
                mvwprintw(cp_win, 25, 3, "[Error] Cannot change directory to '%s'!              ", target_dir);
                wrefresh(cp_win);
                sleep(1);
            }
            continue;
        }

        build_path(cwd, src_input, resolved_src, sizeof(resolved_src));

        FILE *src_file = fopen(resolved_src, "rb");
        if (src_file == NULL) {
            mvwprintw(cp_win, 25, 3, "[Error] Cannot open source file!                      ");
            wrefresh(cp_win);
            sleep(1);
            continue;
        }

        // Step 2: Choose destination directory or file path
        int dest_done = 0;
        int cancelled = 0;

        while (!dest_done) {
            werase(cp_win);
            box(cp_win, 0, 0);
            mvwprintw(cp_win, 0, 4, " Copy File (CP) ");

            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                mvwprintw(cp_win, 2, 3, "Current Path: %s", cwd);
            }

            mvwprintw(cp_win, 3, 3, "Selected Source: %s", resolved_src);
            mvwprintw(cp_win, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

            max_name_len = 0;
            n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
            draw_dir_items(cp_win, win_width, 6, 3, items, n_items, max_name_len);

            mvwprintw(cp_win, 22, 3, "Step 2: Type 'cd <dir>' to move, or enter destination folder/path:");
            mvwprintw(cp_win, 23, 3, "Dest   > ");
            wrefresh(cp_win);

            status = read_line_with_tab(cp_win, 23, 12, 75, dest_input, sizeof(dest_input), items, n_items);
            if(status == 0){
            delwin(cp_win);
            run_cp(); // Restart the function to refresh the window and items
            return;
            }
            if (status == -1) {
                fclose(src_file);
                delwin(cp_win);
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
                    mvwprintw(cp_win, 25, 3, "[Error] Cannot change directory to '%s'!              ", target_dir);
                    wrefresh(cp_win);
                    sleep(1);
                }
                continue;
            }

            build_path(cwd, dest_input, resolved_dest, sizeof(resolved_dest));

            char final_dest[1024];
            struct stat st;
            if (stat(resolved_dest, &st) == 0 && S_ISDIR(st.st_mode)) {
                char *filename = strrchr(resolved_src, '/');
                if (filename != NULL) {
                    filename++;
                } else {
                    filename = resolved_src;
                }
                snprintf(final_dest, sizeof(final_dest), "%s/%s", resolved_dest, filename);
            } else {
                snprintf(final_dest, sizeof(final_dest), "%s", resolved_dest);
            }

            FILE *dest_file = fopen(final_dest, "wb");
            if (dest_file == NULL) {
                mvwprintw(cp_win, 25, 3, "[Error] Cannot create destination file!                 ");
                wrefresh(cp_win);
                sleep(1);
                continue;
            }

            char buffer[8192];
            size_t bytesRead;
            while ((bytesRead = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
                fwrite(buffer, 1, bytesRead, dest_file);
            }

            fclose(dest_file);
            mvwprintw(cp_win, 25, 3, "[Success] File copied successfully!                     ");
            wrefresh(cp_win);
            sleep(1);
            dest_done = 1;
        }

        fclose(src_file);

        if (cancelled) {

            break;
        }
    }

    delwin(cp_win);
}