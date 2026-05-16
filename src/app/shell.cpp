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

#include "shell.h"

#include <iostream>
#include <string>
#include <vector>

#include "command_line.h"
#include "commands.h"
#include "ext_path.h"
#include "ext2read.h"
#include "line_editor.h"
#include "session.h"

using namespace std;

namespace {

void show_shell_help()
{
    cout << "Commands:" << endl;
    cout << "  help" << endl;
    cout << "  pwd" << endl;
    cout << "  cd <extDir>" << endl;
    cout << "  ls [extPath]" << endl;
    cout << "  lsl [extPath]" << endl;
    cout << "  cat <extPath>" << endl;
    cout << "  cp <extPath> <localPath>" << endl;
    cout << "  size <extPath>" << endl;
    cout << "  mode <extPath>" << endl;
    cout << "  ctime <extPath>" << endl;
    cout << "  mtime <extPath>" << endl;
    cout << "  atime <extPath>" << endl;
    cout << "  exit" << endl;
    cout << "  quit" << endl;
}

bool is_ext_path_command(const string &name)
{
    return name == "ls" || name == "lsl" || name == "cat" || name == "cp" ||
           name == "size" || name == "mode" || name == "ctime" ||
           name == "mtime" || name == "atime";
}

void print_path_not_found()
{
    cout << "ERR:Ext Path Not found" << endl;
    cout << "*Please make sure path in selected ext partition exist." << endl;
}

void execute_shell_command(Session &session, const vector<string> &tokens)
{
    const string &name = tokens[0];

    if (name == "help") {
        show_shell_help();
        return;
    }

    if (name == "pwd") {
        cout << session.cwd() << endl;
        return;
    }

    if (name == "cd") {
        if (tokens.size() < 2) {
            cout << "bad parameter" << endl;
            cout << "cd requires extDir." << endl;
            return;
        }

        string resolved = session.resolve_shell_path(tokens[1]);
        Ext2File *file = resolve_ext_path(session.selected_partition(), resolved);
        if (!file) {
            print_path_not_found();
            return;
        }

        if (!EXT2_S_ISDIR(file->inode.i_mode)) {
            cout << "ERR:Not a directory" << endl;
            return;
        }

        session.set_cwd(tokens[1]);
        return;
    }

    if (!is_ext_path_command(name)) {
        if (tokens.size() == 1) {
            string resolved = session.resolve_shell_path(name);
            Ext2File *file = resolve_ext_path(session.selected_partition(), resolved);
            if (file && EXT2_S_ISDIR(file->inode.i_mode)) {
                session.set_cwd(name);
                return;
            }
        }

        cout << "bad parameter" << endl;
        cout << "The specified command option does not exist." << endl;
        return;
    }

    if (name != "ls" && name != "lsl" && tokens.size() < 2) {
        cout << "bad parameter" << endl;
        cout << name << " requires extPath." << endl;
        return;
    }

    if (name == "cp" && tokens.size() < 3) {
        cout << "bad parameter" << endl;
        cout << "cp requires localPath." << endl;
        return;
    }

    CommandRequest request;
    request.name = name;

    if (name == "ls" || name == "lsl") {
        request.ext_path = session.resolve_shell_path(tokens.size() >= 2 ? tokens[1] : "");
    } else if (tokens.size() >= 2) {
        request.ext_path = session.resolve_shell_path(tokens[1]);
    }

    if (tokens.size() >= 3) {
        request.local_path = tokens[2];
    }

    execute_command(session.selected_partition(), request);
}

} // namespace

int run_shell(Session &session)
{
    string line;

    while (true) {
        string prompt = "crext:" + session.cwd() + "> ";

        if (!read_shell_line(prompt, line)) {
            return 0;
        }

        ParsedLine parsed = parse_command_line(line);
        if (parsed.kind == ParsedLineKind::Empty) {
            continue;
        }

        if (parsed.kind == ParsedLineKind::ParseError) {
            cout << "bad parameter" << endl;
            cout << parsed.error << endl;
            continue;
        }

        const string &name = parsed.tokens[0];
        if (name == "exit" || name == "quit") {
            return 0;
        }

        execute_shell_command(session, parsed.tokens);
    }
}
