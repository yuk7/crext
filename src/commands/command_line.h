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

#ifndef CREXT_COMMANDS_COMMAND_LINE_H
#define CREXT_COMMANDS_COMMAND_LINE_H

#include <string>
#include <vector>

enum class ParsedLineKind {
    Empty,
    Tokens,
    ParseError,
};

struct ParsedLine {
    ParsedLineKind kind = ParsedLineKind::Empty;
    std::vector<std::string> tokens;
    std::string error;
};

ParsedLine parse_command_line(const std::string &line);

#endif // CREXT_COMMANDS_COMMAND_LINE_H
