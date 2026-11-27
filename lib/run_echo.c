#include "main.h"

static void do_echo(char *text, int opt_n);

void run_echo() {
    WINDOW *echo_win;
    int highlight = 1;
    int c;

    // Option status
    int opt_n = 0;

    // Text input buffer
    char input_buf[81] = "";
    int input_idx = 0;

    // Create a pop-up window
    echo_win = recreate_menu_win(NULL);

    if (echo_win == NULL) {
        return;
    }



    while (1) {//사용자가 방향키를 누르거나 문자 입력할 때마다 화면을 다시그려야하기 때문
        werase(echo_win);// 기존 화면데이터 지우기
        box(echo_win, 0, 0);

        mvwprintw(echo_win, 0, 4,
                  " echo - Display Text ");

        mvwprintw(echo_win, 2, 3,
                  "Toggle Options (Press Space to select):");

        // 1. Option: -n
        if (highlight == 1)
            wattron(echo_win, A_REVERSE);

        mvwprintw(echo_win, 4, 5,
                  "[%s] -n No trailing newline",
                  opt_n ? "X" : " ");

        if (highlight == 1)
            wattroff(echo_win, A_REVERSE);

        // 2. Text input
        mvwprintw(echo_win, 6, 3,
                  "Enter the text to print:");

        if (highlight == 2)
            wattron(echo_win, A_REVERSE);

        mvwprintw(echo_win, 7, 3, "Text > ");

        if (highlight == 2)
            wattroff(echo_win, A_REVERSE);

        mvwprintw(echo_win, 7, 10,
                  "%s", input_buf);

        // 3. Execute button
        if (highlight == 3)
            wattron(echo_win, A_REVERSE);

        mvwprintw(echo_win, 10, 15,
                  " Enter - Run ");

        if (highlight == 3)
            wattroff(echo_win, A_REVERSE);

        mvwprintw(echo_win, 13, 3,
                  "Up/Down Move | Space Toggle | Esc Go Back");

        if (highlight == 2) {
            wmove(echo_win, 7, 10 + input_idx);
        }

        wrefresh(echo_win);

        // 4. Handle keyboard input
        c = wgetch(echo_win);

        switch (c) {
            case KEY_UP:
                if (highlight > 1)
                    highlight--;
                break;

            case KEY_DOWN:
                if (highlight < 3)
                    highlight++;
                break;

            case KEY_BACKSPACE:
            case 127:
            case '\b':
                if (highlight == 2 && input_idx > 0) {
                    input_idx--;
                    input_buf[input_idx] = '\0';
                }
                break;

            case '\n':
            case KEY_ENTER:
                if (highlight == 2) {
                    highlight = 3;
                } else if (highlight == 3) {

                    // 5. Display the execution result
                    endwin();

                    printf("\n[echo execution result]\n");
                    printf("----------------------------------------\n");

                    fflush(stdout);

                    // Execute echo directly
                    do_echo(input_buf, opt_n);

                    printf("[END]\n");
                    printf("----------------------------------------\n");
                    printf("Press Enter to continue...");

                    fflush(stdout);

                    while ((c = getchar()) != '\n' && c != EOF) {
                    }

                    // Return to ncurses mode
                    refresh();
                    touchwin(echo_win);
                }
                break;

            case 27:
                // ESC: return to main menu
                delwin(echo_win);
                return;

            default:
                if (highlight == 1 && c == ' ') {
                    opt_n = !opt_n;
                } else if (highlight == 2 &&
                           c >= 32 && c <= 126 &&
                           input_idx < sizeof(input_buf) - 1) {

                    input_buf[input_idx++] = (char)c;
                    input_buf[input_idx] = '\0';
                }
                break;
        }
    }
}

// Print text to standard output
static void do_echo(char *text, int opt_n) {
    char output[82];
    int len;

    // 1. Handle the -n option
    if (opt_n) {
        snprintf(output, sizeof(output), "%s", text);
    } else {
        snprintf(output, sizeof(output), "%s\n", text);
    }

    // 2. Write the text to stdout
    len = strlen(output);

    ssize_t result = write(1, output, len);

    if (result == -1) {
        perror("echo");
    } else if (result != len) {
        fprintf(stderr, "[Error] Incomplete write\n");
    }
}