#include "main.h"
#include <fcntl.h>

#define TAIL_DEFAULT_LINES 10

// 화면 상단(제목, 파일 정보) 그리기
static void tail_draw_header(WINDOW *win, const char *filename, int n_lines) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 4, " Tail of File (TAIL) ");
    mvwprintw(win, 1, 3, "File: %s  (-n %d)", filename, n_lines);
}

// 파일 이름과 줄 수(-n) 입력
// return: 1 = 입력 완료, -1 = Esc
static int tail_input(WINDOW **pwin, char *filename, size_t fsize, int *n_lines) {
    char cwd[PATH_MAX_LEN];
    char num[16];
    int h, w;

    while (1) {
        getmaxyx(*pwin, h, w);
        werase(*pwin);
        box(*pwin, 0, 0);
        mvwprintw(*pwin, 0, 4, " Tail of File (TAIL) ");

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            mvwprintw(*pwin, 2, 3, "Current Path: %s", cwd);
        }
        mvwprintw(*pwin, 4, 3, "--- [ Current Directory (Folders & Files) ] ---");

        CdFileItem items[CD_MAX_ITEMS];
        int max_name_len = 0;
        int n_items = load_current_dir_items(items, CD_MAX_ITEMS, &max_name_len);
        draw_dir_items(*pwin, w, 4, 3, items, n_items, max_name_len);

        mvwprintw(*pwin, h - 5, 3, "Enter file name and line count (empty = %d, Esc to exit)",
                  TAIL_DEFAULT_LINES);
        mvwprintw(*pwin, h - 4, 3, "File  > ");
        mvwprintw(*pwin, h - 3, 3, "-n    > ");
        wrefresh(*pwin);

        // 1. 파일 이름 입력 (Tab 자동완성)
        int st = read_line_with_tab(*pwin, h - 4, 11, 40, filename, fsize, items, n_items);
        if (st == -1) return -1;
        if (st == 0) { *pwin = recreate_menu_win(*pwin); continue; }
        if (filename[0] == '\0') continue;

        // 2. 줄 수 입력
        st = read_line_with_tab(*pwin, h - 3, 11, 10, num, sizeof(num), NULL, 0);
        if (st == -1) return -1;
        if (st == 0) { *pwin = recreate_menu_win(*pwin); continue; }

        *n_lines = (num[0] == '\0') ? TAIL_DEFAULT_LINES : atoi(num);
        if (*n_lines < 0) *n_lines = TAIL_DEFAULT_LINES;
        return 1;
    }
}

// 파일 끝에서부터 거꾸로 읽으며 '\n'을 세어, 마지막 n_lines 줄이 시작되는 오프셋을 반환
// return: 시작 오프셋, -1 = 에러
static off_t tail_find_start(int fd, int n_lines) {
    char buf[1024];
    off_t size = lseek(fd, 0, SEEK_END);
    if (size < 0) return -1;
    if (n_lines == 0) return size;

    off_t pos = size;
    int count = 0;

    while (pos > 0) {
        ssize_t chunk = (pos > (off_t)sizeof(buf)) ? (ssize_t)sizeof(buf) : (ssize_t)pos;
        pos -= chunk;

        lseek(fd, pos, SEEK_SET);
        if (read(fd, buf, chunk) != chunk) return -1;

        for (ssize_t i = chunk - 1; i >= 0; i--) {
            // 파일 맨 끝의 '\n'은 마지막 줄의 끝이므로 세지 않음
            if (buf[i] == '\n' && pos + i != size - 1) {
                count++;
                if (count == n_lines) {
                    return pos + i + 1;   // '\n' 다음 바이트부터 출력
                }
            }
        }
    }

    return 0;   // 파일 줄 수가 n_lines보다 적으면 처음부터 출력
}

// fd의 현재 위치부터 최대 max_lines 줄 출력 (화면이 차면 키 입력 후 다음 페이지)
static void tail_print(WINDOW *win, const char *filename, int fd, int max_lines) {
    int h, w;
    getmaxyx(win, h, w);

    char buf[1024];
    ssize_t n = 0;
    int lines = 0;
    int row = 3, col = 3;

    tail_draw_header(win, filename, max_lines);

    while (lines < max_lines && (n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n && lines < max_lines; i++) {
            unsigned char c = (unsigned char)buf[i];

            if (c == '\n') {
                lines++;
                row++;
                col = 3;
            } else if (col < w - 3) {          // 화면 폭을 넘는 부분은 잘라냄
                if (c == '\t') c = ' ';
                mvwaddch(win, row, col++, c);
            }

            // 화면이 가득 차면 다음 페이지
            if (row >= h - 2 && lines < max_lines) {
                mvwprintw(win, h - 2, 3, "-- More -- (press any key)");
                wrefresh(win);
                wgetch(win);
                tail_draw_header(win, filename, max_lines);
                row = 3;
            }
        }
    }

    if (n < 0) {
        mvwprintw(win, h - 2, 3, "Read error. Press any key.");
    } else {
        mvwprintw(win, h - 2, 3, "End of output. Press any key.");
    }
    wrefresh(win);
    wgetch(win);
}

void run_tail() {
    WINDOW *win = recreate_menu_win(NULL);
    char filename[256];
    int n_lines;

    while (tail_input(&win, filename, sizeof(filename), &n_lines) == 1) {
        int h, w;
        getmaxyx(win, h, w);
        (void)w;

        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            mvwprintw(win, h - 2, 3, "Cannot open file: %s", filename);
            wrefresh(win);
            wgetch(win);
            continue;
        }

        off_t start = tail_find_start(fd, n_lines);
        if (start < 0) {
            mvwprintw(win, h - 2, 3, "Read error: %s", filename);
            wrefresh(win);
            wgetch(win);
            close(fd);
            continue;
        }

        lseek(fd, start, SEEK_SET);
        tail_print(win, filename, fd, n_lines);
        close(fd);
    }

    delwin(win);
}
