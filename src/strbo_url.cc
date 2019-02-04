/*
 * Copyright (C) 2023  T+A elektroakustik GmbH & Co. KG
 *
 * This file is part of T+A StrBo-URL.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "strbo_url.hh"
#include "strbo_url_helpers.hh"

#include <limits>

const std::string StrBoUrl::Location::valid_characters =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$-_.~+!*'(),;/?:@=&%";

const std::string StrBoUrl::Location::safe_characters =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789$-_.~";

void StrBoUrl::for_each_url_encoded(const std::string &src,
                                    const std::function<void(const char *, size_t)> &apply)
{
    for(const char &ch : src)
    {
        if(StrBoUrl::Location::safe_characters.find(ch) != std::string::npos)
            apply(&ch, 1);
        else
        {
            char buffer[4];
            const int len = snprintf(buffer, sizeof(buffer), "%%%02X", ch);

            apply(buffer, len);
        }
    }
}

static bool decode(const char ch1, const char ch2, uint8_t &out)
{
    if(ch1 >= '0' && ch1 <= '9')
        out = static_cast<uint8_t>(ch1 - '0') << 4;
    else if(ch1 >= 'A' && ch1 <= 'F')
        out = static_cast<uint8_t>(ch1 - 'A' + 10) << 4;
    else
        return false;

    if(ch2 >= '0' && ch2 <= '9')
        out |= static_cast<uint8_t>(ch2 - '0');
    else if(ch2 >= 'A' && ch2 <= 'F')
        out |= static_cast<uint8_t>(ch2 - 'A' + 10);
    else
        return false;

    return true;
}

void StrBoUrl::for_each_url_decoded(const std::string &src,
                                    const std::function<void(char)> &apply,
                                    const std::function<void(std::string &&error)> &on_decode_error)
{
    for(size_t i = 0; i < src.length(); ++i)
    {
        const char ch = src[i];

        if(ch != '%')
        {
            apply(ch);
            continue;
        }

        if(i + 3 <= src.length())
        {
            uint8_t out;

            if(decode(src[i + 1], src[i + 2], out))
            {
                i += 2;
                apply(out);
                continue;
            }

            if(on_decode_error != nullptr)
            {
                std::string error("Invalid URL-encoding \"");
                error += src[i + 0];
                error += src[i + 1];
                error += src[i + 2];
                error += "\" in URL \"";
                error += src;
                error += '"';
                on_decode_error(std::move(error));
            }
        }
        else if(on_decode_error != nullptr)
        {
            std::string error("URL too short for last code: \"");
            error += src;
            error += '"';
            on_decode_error(std::move(error));
        }

        break;
    }
}

std::string::size_type
StrBoUrl::Parse::extract_field(const std::string &url, size_t offset,
                               const char separator, FieldPolicy policy,
                               const std::function<void(const char *error_message)> &on_error)
{
    const auto end_of_field = url.find(separator, offset);

    if(end_of_field == std::string::npos)
    {
        switch(policy)
        {
          case FieldPolicy::FIELD_OPTIONAL:
            break;

          case FieldPolicy::MAY_BE_EMPTY:
          case FieldPolicy::MUST_NOT_BE_EMPTY:
            {
                std::string temp("No '");
                temp += separator;
                temp += "' found";
                on_error(temp.c_str());
            }

            break;
        }
    }
    else
    {
        switch(policy)
        {
          case FieldPolicy::FIELD_OPTIONAL:
          case FieldPolicy::MAY_BE_EMPTY:
            break;

          case FieldPolicy::MUST_NOT_BE_EMPTY:
            if(end_of_field <= offset)
            {
                on_error("Component empty");
                return std::string::npos;
            }

            break;
        }
    }

    return end_of_field;
}

static StrBoUrl::ObjectIndex
parse_item_position(const std::string &url, size_t offset,
                    size_t expected_end, bool expecting_zero_terminator,
                    const std::function<void(const char *error_message)> &on_error)
{
    if(offset >= expected_end)
    {
        on_error("Component empty");
        return StrBoUrl::ObjectIndex();
    }

    char *endptr = nullptr;
    unsigned long long temp = strtoull(&url[offset], &endptr, 10);

    if(*endptr != '\0' && expecting_zero_terminator)
    {
        on_error("Component with trailing junk");
        return StrBoUrl::ObjectIndex();
    }

    if((temp == std::numeric_limits<unsigned long long>::max() && errno == ERANGE) ||
       temp > std::numeric_limits<uint32_t>::max())
    {
        on_error("Component out of range");
        return StrBoUrl::ObjectIndex();
    }

    return StrBoUrl::ObjectIndex(temp);
}

StrBoUrl::ObjectIndex
StrBoUrl::Parse::item_position(const std::string &url, size_t offset,
                               size_t expected_end,
                               const std::function<void(const char *error_message)> &on_error)
{
    return parse_item_position(url, offset, expected_end, false, on_error);
}

StrBoUrl::ObjectIndex
StrBoUrl::Parse::item_position(const std::string &url, size_t offset,
                               const std::function<void(const char *error_message)> &on_error)
{
    return parse_item_position(url, offset, url.length(), true, on_error);
}
