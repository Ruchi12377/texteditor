/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    // プログラムの終了時に関数を呼び出す
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    // インプットフラグ
    // ICRNL Ctrl-Mを無効にする (carriage retrun new line)
    // IXION Ctrl-SとCtrl-Qを無効にする
    raw.c_iflag &= ~(ICRNL | IXON);
    // アウトプットフラグ
    // OPOST 出力後の後処理(\nから\r\nへの変換)をオフにする
    raw.c_oflag &= ~(OPOST);
    // データビット数を8にする
    raw.c_cflag |= (CS8);
    // ECHO 表示をオフにする
    // ICANON カノニカルモードをオフにする
    // IEXTEN Ctrl-Vを無効にする、MacでCtrl-Oを無効にする
    // ISIG Ctrl-Cなどを無効にする
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** init ***/

int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read"); 
        // 文字が制御文字かどうかをテストする
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            // 97 ('a')
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == CTRL_KEY('q')) break;
    }
    return 0;
}