/*
 * Copyright (c) 2015-2026 yuk7
 * Author: yuk7 <yukx00@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "line_editor.h"

#include <iostream>

#if defined(_WIN32) || defined(WIN32)
#include <io.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

namespace {

#if !defined(_WIN32) && !defined(WIN32)
class TerminalRawMode {
public:
    TerminalRawMode()
        : enabled(false)
    {
        if (!isatty(STDIN_FILENO)) {
            return;
        }

        if (tcgetattr(STDIN_FILENO, &original) != 0) {
            return;
        }

        termios raw = original;
        raw.c_lflag &= static_cast<unsigned long>(~(ICANON | ECHO));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        enabled = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0;
    }

    ~TerminalRawMode()
    {
        if (enabled) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
        }
    }

    bool is_enabled() const
    {
        return enabled;
    }

private:
    termios original;
    bool enabled;
};
#endif

void redraw_line(const string &prompt, const string &line, size_t cursor)
{
    cout << "\r" << prompt << line << "\x1b[K";
    if (line.size() > cursor) {
        cout << "\x1b[" << (line.size() - cursor) << "D";
    }
    cout.flush();
}

} // namespace

bool read_shell_line(const string &prompt, string &line)
{
#if defined(_WIN32) || defined(WIN32)
    cout << prompt;
    cout.flush();
    return static_cast<bool>(getline(cin, line));
#else
    if (!isatty(STDIN_FILENO)) {
        cout << prompt;
        cout.flush();
        return static_cast<bool>(getline(cin, line));
    }

    TerminalRawMode raw_mode;
    if (!raw_mode.is_enabled()) {
        cout << prompt;
        cout.flush();
        return static_cast<bool>(getline(cin, line));
    }

    line.clear();
    size_t cursor = 0;
    cout << prompt;
    cout.flush();

    while (true) {
        unsigned char ch;
        ssize_t count = read(STDIN_FILENO, &ch, 1);
        if (count == 0) {
            cout << endl;
            return false;
        }
        if (count < 0) {
            return false;
        }

        if (ch == '\r' || ch == '\n') {
            cout << endl;
            return true;
        }

        if (ch == 3) {
            line.clear();
            cout << "^C" << endl;
            return true;
        }

        if (ch == 4) {
            if (line.empty()) {
                cout << endl;
                return false;
            }
            continue;
        }

        if (ch == 127 || ch == 8) {
            if (cursor > 0) {
                line.erase(cursor - 1, 1);
                --cursor;
                redraw_line(prompt, line, cursor);
            }
            continue;
        }

        if (ch == 27) {
            unsigned char seq[3] = {0, 0, 0};
            if (read(STDIN_FILENO, &seq[0], 1) != 1) {
                continue;
            }
            if (read(STDIN_FILENO, &seq[1], 1) != 1) {
                continue;
            }

            if (seq[0] == '[') {
                if (seq[1] == 'D') {
                    if (cursor > 0) {
                        --cursor;
                        cout << "\x1b[D";
                        cout.flush();
                    }
                } else if (seq[1] == 'C') {
                    if (cursor < line.size()) {
                        ++cursor;
                        cout << "\x1b[C";
                        cout.flush();
                    }
                } else if (seq[1] == '3') {
                    if (read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~' &&
                        cursor < line.size()) {
                        line.erase(cursor, 1);
                        redraw_line(prompt, line, cursor);
                    }
                } else if (seq[1] == 'H') {
                    cursor = 0;
                    redraw_line(prompt, line, cursor);
                } else if (seq[1] == 'F') {
                    cursor = line.size();
                    redraw_line(prompt, line, cursor);
                }
            }
            continue;
        }

        if (ch < 32) {
            continue;
        }

        line.insert(cursor, 1, static_cast<char>(ch));
        ++cursor;
        redraw_line(prompt, line, cursor);
    }
#endif
}
