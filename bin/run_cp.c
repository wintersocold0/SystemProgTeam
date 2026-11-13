#include "main.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

void run_cp() {
    WINDOW *cp_win;
    char src_path[256] = "";
    char dest_path[256] = "";
    char cwd[1025] = "";
    
    // Create a pop-up window (height, width, start_y, start_x)
    cp_win = newwin(30, 100, 3, 25);
    keypad(cp_win, TRUE);
    
    while (1) {
        werase(cp_win);
        box(cp_win, 0, 0);
        mvwprintw(cp_win, 0, 4, " Copy File (CP) ");
        
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(cp_win, 3, 3, "Current Path: %s", cwd);
        }
        
        // Instructions and input prompts (Added Esc to exit hint)
        mvwprintw(cp_win, 6, 3, "Enter source file path (or type 'exit' / press Esc to quit):");
        mvwprintw(cp_win, 7, 3, "Source > ");
        
        mvwprintw(cp_win, 10, 3, "Enter destination path or directory:");
        mvwprintw(cp_win, 11, 3, "Dest   > ");
        wrefresh(cp_win);

        // 1. Get Source Path Input with Esc support
        int src_idx = 0;
        memset(src_path, 0, sizeof(src_path));
        
        while (1) {
            mvwprintw(cp_win, 7, 12, "%-50s", ""); // Clear old text
            mvwprintw(cp_win, 7, 12, "%s", src_path);
            wmove(cp_win, 7, 12 + strlen(src_path));
            wrefresh(cp_win);

            int ch = wgetch(cp_win);

            if (ch == 27) { // 27 is the ASCII code for ESC
                delwin(cp_win);
                return;
            } 
            else if (ch == '\n' || ch == KEY_ENTER) {
                break;
            } 
            else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (src_idx > 0) {
                    src_idx--;
                    src_path[src_idx] = '\0';
                }
            } 
            else if (ch >= 32 && ch <= 126 && src_idx < sizeof(src_path) - 1) {
                src_path[src_idx++] = (char)ch;
                src_path[src_idx] = '\0';
            }
        }
        
        // Exit check via string
        if (strcmp(src_path, "exit") == 0 || strcmp(src_path, "EXIT") == 0) {
            break;
        }

        // 2. Get Destination Path Input with Esc support
        int dest_idx = 0;
        memset(dest_path, 0, sizeof(dest_path));

        while (1) {
            mvwprintw(cp_win, 11, 12, "%-50s", ""); // Clear old text
            mvwprintw(cp_win, 11, 12, "%s", dest_path);
            wmove(cp_win, 11, 12 + strlen(dest_path));
            wrefresh(cp_win);

            int ch = wgetch(cp_win);

            if (ch == 27) { // ESC key
                delwin(cp_win);
                return;
            } 
            else if (ch == '\n' || ch == KEY_ENTER) {
                break;
            } 
            else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                if (dest_idx > 0) {
                    dest_idx--;
                    dest_path[dest_idx] = '\0';
                }
            } 
            else if (ch >= 32 && ch <= 126 && dest_idx < sizeof(dest_path) - 1) {
                dest_path[dest_idx++] = (char)ch;
                dest_path[dest_idx] = '\0';
            }
        }

        if (strcmp(dest_path, "exit") == 0 || strcmp(dest_path, "EXIT") == 0) {
            break;
        }

        // 3. Open source file for reading in binary mode
        FILE *src_file = fopen(src_path, "rb");
        if (src_file == NULL) {
            mvwprintw(cp_win, 16, 3, "[Error] Cannot open source file!                      ");
            wrefresh(cp_win);
            sleep(1);
            continue;
        }

        // 4. Handle destination if it is a directory
        char final_dest[512];
        struct stat st;
        if (stat(dest_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            // If destination is a directory, extract filename and append it
            char *filename = strrchr(src_path, '/');
            if (filename != NULL) {
                filename++; // Skip the '/'
            } else {
                filename = src_path;
            }
            snprintf(final_dest, sizeof(final_dest), "%s/%s", dest_path, filename);
        } else {
            snprintf(final_dest, sizeof(final_dest), "%s", dest_path);
        }

        // 5. Open destination file for writing in binary mode
        FILE *dest_file = fopen(final_dest, "wb");
        if (dest_file == NULL) {
            mvwprintw(cp_win, 16, 3, "[Error] Cannot create destination file!                 ");
            fclose(src_file);
            wrefresh(cp_win);
            sleep(1);
            continue;
        }

        // 6. Copy file content chunk by chunk
        char buffer[8192];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
            fwrite(buffer, 1, bytesRead, dest_file);
        }

        // Close files
        fclose(src_file);
        fclose(dest_file);

        // 7. Show success message
        mvwprintw(cp_win, 16, 3, "[Success] File copied successfully!                     ");
        wrefresh(cp_win);
        
        // Pause briefly before clearing for the next operation
        sleep(1);
    }
    
    delwin(cp_win);
}