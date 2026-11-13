#include "main.h"

#define MAX_ITEMS 256

typedef struct {
    char name[256];
    int is_dir;
} FileItem;

void run_cd() {
    WINDOW *cd_win;
    char cwd[1024];
    
    // Create a pop-up window (height, width, start_y, start_x)
    int win_height = 30;
    int win_width = 100;
    cd_win = newwin(win_height, win_width, 3, 25);
    keypad(cd_win, TRUE);
    box(cd_win, 0, 0);
    mvwprintw(cd_win, 0, 4, " Change Directory (CD) ");
    
    while (1) {
        werase(cd_win);
        box(cd_win, 0, 0);
        mvwprintw(cd_win, 0, 4, " Change Directory (CD) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(cd_win, 3, 3, "Current Path: %s", cwd);
        }
        
        mvwprintw(cd_win, 6, 3, "--- [ Current Directory (Folders & Files) ] ---");

        // 1. Read items under the current directory into the array
        DIR *dir = opendir(".");
        FileItem items[MAX_ITEMS];
        int n_items = 0;
        int max_name_len = 0;

        if (dir != NULL) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL && n_items < MAX_ITEMS) {
                snprintf(items[n_items].name, sizeof(items[n_items].name), "%s", entry->d_name);
                
                struct stat st;
                if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                    items[n_items].is_dir = 1;
                } else {
                    items[n_items].is_dir = 0;
                }

                // Find the longest name length (for auto width calculation)
                int len = strlen(entry->d_name);
                if (len > max_name_len) {
                    max_name_len = len;
                }

                n_items++;
            }
            closedir(dir);
        }

        // 2. Calculate Auto Column Width
        int start_y = 8;
        int start_x = 3;
        int available_width = win_width - 6; // Exclude left and right margins
        
        // Automatically set column width based on the longest name + padding
        int col_width = max_name_len + 5;
        if (col_width < 15) col_width = 20;   // Minimum width
        if (col_width > 40) col_width = 45;   // Limit to maximum width if too long
        
        int max_cols = available_width / col_width;
        if (max_cols < 1) max_cols = 1;
        if (max_cols > 3) max_cols = 3;

        // 3. Display items in columns using the calculated Auto Width
        for (int i = 0; i < n_items && i < 24; ++i) {
            int col = i % max_cols;
            int row = i / max_cols;
            
            int current_y = start_y + row + 2;
            int current_x = start_x + (col * col_width);

            // Use format string based on dynamic col_width
            char format_str[32];
            snprintf(format_str, sizeof(format_str), "%%-%ds", col_width - 2);

            mvwprintw(cd_win, current_y, current_x, format_str, items[i].name);
        }

        // 4. Request target folder name from the user (includes Tab completion)
        int input_y = 22;
        mvwprintw(cd_win, input_y, 3, "Enter the path name (Press Esc to exit):                        ");
        mvwprintw(cd_win, input_y + 1, 3, "Path > ");
        wrefresh(cd_win);

        char input_buf[256] = "";
        int buf_idx = 0;
        int prompt_y = input_y + 1;
        int prompt_x = 10;

        while (1) {
            mvwprintw(cd_win, prompt_y, prompt_x, "%-50s", ""); 
            mvwprintw(cd_win, prompt_y, prompt_x, "%s", input_buf);
            wmove(cd_win, prompt_y, prompt_x + strlen(input_buf));
            wrefresh(cd_win);

            int ch = wgetch(cd_win);

            if (ch == '\n' || ch == KEY_ENTER) {
                break; 
            } 
            else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (buf_idx > 0) {
                    buf_idx--;
                    input_buf[buf_idx] = '\0';
                }
            } 
            else if (ch == '\t') { 
                // --- [ TAB Auto-complete ] ---
                char match[256] = "";
                int match_count = 0;

                for (int i = 0; i < n_items; ++i) {
                    if (strncmp(items[i].name, input_buf, strlen(input_buf)) == 0) {
                        if (strcmp(items[i].name, ".") == 0 || strcmp(items[i].name, "..") == 0) continue;
                        snprintf(match, sizeof(match), "%s", items[i].name);
                        match_count++;
                    }
                }

                if (match_count > 0) {
                    snprintf(input_buf, sizeof(input_buf), "%s", match);
                    buf_idx = strlen(input_buf);
                }
            } 
            else if (ch >= 32 && ch <= 126 && buf_idx < sizeof(input_buf) - 1) {
                input_buf[buf_idx++] = (char)ch;
                input_buf[buf_idx] = '\0';
            }
            else if (ch == 27){
                delwin(cd_win);
                return;
            }
        }
        
        if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "EXIT") == 0) {
            break;
        }

        // 5. Change directory using chdir()
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