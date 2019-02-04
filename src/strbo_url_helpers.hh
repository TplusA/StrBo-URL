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

#ifndef STRBO_URL_HELPERS_HH
#define STRBO_URL_HELPERS_HH

#include <string>
#include <functional>

namespace StrBoUrl
{

void for_each_url_encoded(const std::string &src,
                          const std::function<void(const char *, size_t)> &apply);

void for_each_url_decoded(const std::string &src,
                          const std::function<void(char)> &apply,
                          const std::function<void(std::string &&error)> &on_decode_error);

class ObjectIndex;

namespace Parse
{

enum class FieldPolicy
{
    FIELD_OPTIONAL,
    MAY_BE_EMPTY,
    MUST_NOT_BE_EMPTY,
};

std::string::size_type
extract_field(const std::string &url, size_t offset, const char separator,
              FieldPolicy policy,
              const std::function<void(const char *error_message)> &on_error);

StrBoUrl::ObjectIndex
item_position(const std::string &url, size_t offset, size_t expected_end,
              const std::function<void(const char *error_message)> &on_error);

StrBoUrl::ObjectIndex
item_position(const std::string &url, size_t offset,
              const std::function<void(const char *error_message)> &on_error);

}

}

#endif /* !STRBO_URL_HELPERS_HH */
