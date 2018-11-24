#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>
/*                      Luke Holman - September 22, 2018
                        The Game of Life                            */
struct winsize size;
/*             size.ws_row is the number of rows,
               size.ws_col is the number of columns.                */
struct dot
{
    int x;
    int y;
};
struct termios orig_termios;

void initalize_arr(), draw_frame(), draw_cursor(), move_dots(), print_buff();
int key_hit(), check_neighbors();
char getch();

int cursorX = 0, cursorY = 0;
bool edit_mode = true;
bool mass_edit = false;

int main()
{
    system("clear");
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    int rowSize = size.ws_row - 1, colSize = size.ws_col;
    int buff[size.ws_row][size.ws_col];
    int dots[size.ws_row][size.ws_col];
    initalize_arr(rowSize, colSize, dots);
    cursorX = colSize / 2;
    cursorY = rowSize / 2;
    do
    {
        draw_frame(rowSize, colSize, buff, dots);
        if (edit_mode)
        {
            draw_cursor(rowSize, colSize, buff, dots);
        }
        else
        {
            do
            {
                move_dots(rowSize, colSize, dots);
                print_buff(rowSize, colSize, buff);
                draw_frame(rowSize, colSize, buff, dots);
                usleep(200000);
            } while (true);
        }
        print_buff(rowSize, colSize, buff);
    } while (key_hit(rowSize, colSize, dots));
    return 0;
}

void move_dots(int rowSize,
               int colSize,
               int dots[rowSize][colSize])
{
    struct dot dot_i;
    int tDots[rowSize][colSize];
    int row = 0, col = 0;
    for (; row < rowSize; row++)
    {
        for (; col < colSize; col++)
            tDots[row][col] = dots[row][col];
        col = 0;
    }
    row = 0, col = 0;
    for (; row < rowSize - 1; row++)
    {
        for (; col < colSize; col++)
        {
            dot_i.x = col;
            dot_i.y = row;
            // 1) Any live cell with fewer than two live neighbors dies
            if (check_neighbors(rowSize, colSize, tDots, dot_i) < 2)
            {
                dots[row][col] = 0;
            }
            // 2) Any live cell with more than three live neighbors dies
            if (check_neighbors(rowSize, colSize, tDots, dot_i) > 3)
            {
                dots[row][col] = 0;
            }
            // 3) Any live cell with two or three live neighbors lives on
            if (tDots[row][col])
            {
                if (check_neighbors(rowSize, colSize, tDots, dot_i) == 3 ||
                    check_neighbors(rowSize, colSize, tDots, dot_i) == 2)
                {
                    dots[row][col] = 1;
                }
            }
            // 4) Any dead cell with exactly three live neighbors becomes live
            if (!tDots[row][col])
            {
                if (check_neighbors(rowSize, colSize, tDots, dot_i) == 3)
                {
                    dots[row][col] = 1;
                }
            }
        }
        col = 0;
    }
}

int check_neighbors(int rowSize,
                    int colSize,
                    int dots[rowSize][colSize],
                    struct dot dot_i)
{
    int neighbors = 0, x = dot_i.x, y = dot_i.y;
    if (y > 0 && y < rowSize)
    {
        if (x > 0 && x < colSize)
        {
            x--, y--;
            if (dots[y][x]) neighbors++; x++;
            if (dots[y][x]) neighbors++; x++;
            if (dots[y][x]) neighbors++; y++;
            if (dots[y][x]) neighbors++; y++;
            if (dots[y][x]) neighbors++; x--;
            if (dots[y][x]) neighbors++; x--;
            if (dots[y][x]) neighbors++; y--;
            if (dots[y][x]) neighbors++;
        }
    }
    return neighbors;
}

int key_hit(int rowSize,
            int colSize,
            int dots[rowSize][colSize])
{
    char c = getch();
    if (c == '\033')
    {                   //            escape sequence
        getch();        //              skip first [
        switch (getch())
        {
            default:
                break;
            case 'A':
            {
                // arrow up          /|
                if (edit_mode)
                {
                    cursorY--;
                    if (!mass_edit) break;
                    if (dots[cursorY][cursorX] == 0) dots[cursorY][cursorX] = 1;
                    else dots[cursorY][cursorX] = 0;
                }
                break;
            }
            case 'B':
            {
                // arrow down        \|
                if (edit_mode)
                {
                    cursorY++;
                    if (!mass_edit) break;
                    if (dots[cursorY][cursorX] == 0) dots[cursorY][cursorX] = 1;
                    else dots[cursorY][cursorX] = 0;
                }
                break;
            }
            case 'C':
            {
                // arrow right       >>
                if (edit_mode)
                {
                    cursorX++;
                    if (!mass_edit) break;
                    if (dots[cursorY][cursorX] == 0) dots[cursorY][cursorX] = 1;
                    else dots[cursorY][cursorX] = 0;
                }
                break;
            }
            case 'D':
            {
                // arrow left        <<
                if (edit_mode)
                {
                    cursorX--;
                    if (!mass_edit) break;
                    if (dots[cursorY][cursorX] == 0) dots[cursorY][cursorX] = 1;
                    else dots[cursorY][cursorX] = 0;
                }
                break;
            }
        }
    }
    else
    {
        switch (c)
        {
            default:
                break;
            case 10:
            {
                //                   enter
                if (edit_mode) edit_mode = false;
                else edit_mode = true;
                break;
            }
            case 32:
            {
                //                   space
                if (edit_mode)
                {
                    if (dots[cursorY][cursorX] == 0)
                        dots[cursorY][cursorX] = 1;
                    else dots[cursorY][cursorX] = 0;
                }
                break;
            }
            case 109:
            {
                //                    m
                if (edit_mode)
                {
                    if (mass_edit) mass_edit = false;
                    else mass_edit = true;
                }
                break;
            }
        }
        return 1;
    }
    return 0;
}

char getch()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

void draw_cursor(int rowSize,
                 int colSize,
                 int buff[rowSize][colSize],
                 int dots[rowSize][colSize])
{
    int row = 0, col = 0;
    for (; row < rowSize; row++)
    {
        for (; col < colSize; col++)
        {
            if (row == cursorY)
            {
                if (!dots[row][col]) buff[row][col] = 45; //      -
                else buff[row][col] = 204; //          out of range box
            }
            if (col == cursorX)
            {
                if (!dots[row][col]) buff[row][col] = 124; //     |
                else buff[row][col] = 204; //          out of range box
            }
            if (row == cursorY && col == cursorX)
            {
                if (!dots[row][col]) buff[row][col] = 43; //      +
                else buff[row][col] = 204; //          out of range box
            }
        }
        col = 0;
    }
}

void draw_frame(int rowSize,
                int colSize,
                int buff[rowSize][colSize],
                int dots[rowSize][colSize])
{
    int row = 0, col = 0;
    for (; row < size.ws_row; row++)
    {
        for (; col < size.ws_col; col++)
        {
            if (row == 0)
            {
                buff[row][col] = 61; //               =
            }
            else if (row > 0 && row < rowSize - 1)
            {
                if (col == 0)
                {
                    buff[row][col] = 124; //          |
                }
                else if (col > 0 && col < colSize - 1)
                {
                    if (dots[row][col])
                    {
                        buff[row][col] = 204; // out of range box
                    }
                    else
                    {
                        buff[row][col] = 32; //     space
                    }
                }
                else if (col == colSize - 1)
                {
                    buff[row][col] = 124; //          |
                }
            }
            else if (row == rowSize - 1)
            {
                buff[row][col] = 61; //               =
            }
        }
        col = 0;
    }
}

void print_buff(int rowSize,
                int colSize,
                int buff[rowSize][colSize])
{
    system("clear");
    int row = 0, col = 0;
    for (; row < rowSize; row++)
    {
        for (; col < colSize; col++)
            printf("%c", buff[row][col]);
        col = 0;
        printf("\n");
    }
}

void initalize_arr(int rowSize,
                   int colSize,
                   int pInt[rowSize][colSize])
{
    int row = 0, col = 0;
    for (; row < rowSize; row++)
    {
        for (; col < colSize; col++)
            pInt[row][col] = 0;
        col = 0;
    }
}

