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

#include "command_line.h"

#include <cctype>

using namespace std;

ParsedLine parse_command_line(const string &line)
{
    ParsedLine parsed;
    string token;
    bool token_started = false;
    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escaped = false;

    for (char ch : line) {
        if (escaped) {
            token.push_back(ch);
            token_started = true;
            escaped = false;
            continue;
        }

        if (ch == '\\') {
            escaped = true;
            token_started = true;
            continue;
        }

        if (in_single_quote) {
            if (ch == '\'') {
                in_single_quote = false;
            } else {
                token.push_back(ch);
            }
            token_started = true;
            continue;
        }

        if (in_double_quote) {
            if (ch == '"') {
                in_double_quote = false;
            } else {
                token.push_back(ch);
            }
            token_started = true;
            continue;
        }

        if (ch == '\'') {
            in_single_quote = true;
            token_started = true;
            continue;
        }

        if (ch == '"') {
            in_double_quote = true;
            token_started = true;
            continue;
        }

        if (isspace(static_cast<unsigned char>(ch))) {
            if (token_started) {
                parsed.tokens.push_back(token);
                token.clear();
                token_started = false;
            }
            continue;
        }

        token.push_back(ch);
        token_started = true;
    }

    if (escaped) {
        token.push_back('\\');
    }

    if (in_single_quote || in_double_quote) {
        parsed.kind = ParsedLineKind::ParseError;
        parsed.error = "Unterminated quote.";
        return parsed;
    }

    if (token_started) {
        parsed.tokens.push_back(token);
    }

    parsed.kind = parsed.tokens.empty() ? ParsedLineKind::Empty : ParsedLineKind::Tokens;
    return parsed;
}
