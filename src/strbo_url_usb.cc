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

#include "strbo_url_usb.hh"
#include "strbo_url_helpers.hh"

#include <sstream>
#include <algorithm>

std::string USB::LocationKeySimple::str_impl() const
{
    std::string result = scheme_.get_scheme_name() + "://";

    StrBoUrl::for_each_url_encoded(c_.device_,
        [&result] (const char *enc, size_t len) { result.append(enc, len); });

    result += ':';

    StrBoUrl::for_each_url_encoded(c_.partition_,
        [&result] (const char *enc, size_t len) { result.append(enc, len); });

    result += '/';

    StrBoUrl::for_each_url_encoded(c_.path_,
        [&result] (const char *enc, size_t len) { result.append(enc, len); });

    return result;
}

const char *USB::LocationKeySimple::get_error_prefix()
{
    return "Simple USB location key malformed: ";
}

const char *USB::LocationKeySimple::set_url_impl(const std::string &url, size_t offset)
{
    const auto end_of_device = StrBoUrl::Parse::extract_field(
        url, offset, ':', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Device", error_message);
        });

    const auto end_of_partition = StrBoUrl::Parse::extract_field(
        url, offset, '/', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Partition", error_message);
        });

    if(end_of_partition <= end_of_device)
        throw ParsingError(get_error_prefix(), nullptr,
                           "Failed parsing device and partition");

    c_.device_.clear();
    c_.partition_.clear();
    c_.path_.clear();

    StrBoUrl::for_each_url_decoded(url.substr(offset, end_of_device - offset),
        [this] (const char ch) { c_.device_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Device", e.c_str()); });
    StrBoUrl::for_each_url_decoded(url.substr(end_of_device + 1,
                                              end_of_partition - end_of_device - 1),
        [this] (const char ch) { c_.partition_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Partition", e.c_str()); });
    StrBoUrl::for_each_url_decoded(url.substr(end_of_partition + 1),
        [this] (const char ch) { c_.path_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Item name", e.c_str()); });

    is_partition_set_ = true;
    is_path_set_ = true;
    return nullptr;
}

std::string USB::LocationKeyReference::str_impl() const
{
    std::ostringstream os;
    os << scheme_.get_scheme_name() << "://";

    StrBoUrl::for_each_url_encoded(c_.device_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':';

    StrBoUrl::for_each_url_encoded(c_.partition_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << '/';

    StrBoUrl::for_each_url_encoded(c_.reference_point_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << '/';

    StrBoUrl::for_each_url_encoded(c_.item_name_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':' << c_.item_position_.get_object_index();

    return os.str();
}

const char *USB::LocationKeyReference::get_error_prefix()
{
    return "Reference USB location key malformed: ";
}

const char *USB::LocationKeyReference::set_url_impl(const std::string &url, size_t offset)
{
    const auto end_of_device = StrBoUrl::Parse::extract_field(
        url, offset, ':', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Device", error_message);
        });

    const auto end_of_partition = StrBoUrl::Parse::extract_field(
        url, offset, '/', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Partition", error_message);
        });

    if(end_of_partition <= end_of_device)
        throw ParsingError(get_error_prefix(), nullptr,
                           "Failed parsing device and partition");

    const auto end_of_reference = StrBoUrl::Parse::extract_field(
        url, end_of_partition + 1, '/', StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Reference point", error_message);
        });

    const bool is_reference_empty = end_of_reference == end_of_partition + 1;

    const auto end_of_item = StrBoUrl::Parse::extract_field(
        url, end_of_reference + 1, ':',
        is_reference_empty
        ? StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY
        : StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item name", error_message);
        });

    const auto item_position = StrBoUrl::Parse::item_position(
        url, end_of_item + 1,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item position", error_message);
        });

    std::string temp;
    bool is_path = false;

    StrBoUrl::for_each_url_decoded(
        url.substr(end_of_reference + 1, end_of_item - end_of_reference - 1),
        [&temp, &is_path] (const char ch)
        {
            temp += ch;

            if(ch == '/')
                is_path = true;
        },
        [] (std::string &&e) { throw ParsingError(get_error_prefix(), "Item component", e.c_str()); });

    if(is_path)
        throw ParsingError(get_error_prefix(), "Item component", "Component is a path");

    c_.device_.clear();
    c_.partition_.clear();
    c_.reference_point_.clear();
    c_.item_name_ = std::move(temp);
    c_.item_position_ = item_position;

    StrBoUrl::for_each_url_decoded(url.substr(offset, end_of_device - offset),
        [this] (const char ch) { c_.device_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Device", e.c_str()); });
    StrBoUrl::for_each_url_decoded(url.substr(end_of_device + 1,
                                              end_of_partition - end_of_device - 1),
        [this] (const char ch) { c_.partition_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Partition", e.c_str()); });
    StrBoUrl::for_each_url_decoded(url.substr(end_of_partition + 1,
                                              end_of_reference - end_of_partition - 1),
        [this] (const char ch) { c_.reference_point_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Reference point", e.c_str()); });

    is_partition_set_ = true;
    is_reference_point_set_ = true;
    is_item_set_ = true;
    return nullptr;
}

size_t USB::LocationTrace::get_trace_length() const
{

    if(c_.item_name_.empty())
        return 0;

    return 1 + std::count_if(c_.item_name_.begin(), c_.item_name_.end(),
                             [] (const char &ch) { return ch == '/'; });
}

std::string USB::LocationTrace::str_impl() const
{
    std::ostringstream os;
    os << scheme_.get_scheme_name() << "://";

    StrBoUrl::for_each_url_encoded(c_.device_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':';

    StrBoUrl::for_each_url_encoded(c_.partition_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << '/';

    if(!c_.reference_point_.empty())
    {
        StrBoUrl::for_each_url_encoded(c_.reference_point_,
            [&os] (const char *enc, size_t len) { os.write(enc, len); });

        os << '/';
    }

    StrBoUrl::for_each_url_encoded(c_.item_name_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':' << c_.item_position_.get_object_index();

    return os.str();
}

const char *USB::LocationTrace::get_error_prefix()
{
    return "USB location trace malformed: ";
}

const char *USB::LocationTrace::set_url_impl(const std::string &url, size_t offset)
{
    const auto end_of_device = StrBoUrl::Parse::extract_field(
        url, offset, ':', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Device", error_message);
        });

    const auto end_of_partition = StrBoUrl::Parse::extract_field(
        url, offset, '/', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Partition", error_message);
        });

    if(end_of_partition <= end_of_device)
        throw ParsingError(get_error_prefix(), nullptr,
                           "Failed parsing device and partition");

    const auto end_of_reference =
        url.find('/', end_of_partition + 1) != std::string::npos
        ? StrBoUrl::Parse::extract_field(
            url, end_of_partition + 1, '/', StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY,
            [] (const char *error_message)
            {
                throw ParsingError(get_error_prefix(), "Reference point", error_message);
            })
        : end_of_partition;

    const bool is_reference_empty = end_of_reference == end_of_partition;

    const auto end_of_item = StrBoUrl::Parse::extract_field(
        url, end_of_reference + 1, ':',
        is_reference_empty
        ? StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY
        : StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item name", error_message);
        });

    const auto item_position = StrBoUrl::Parse::item_position(
        url, end_of_item + 1,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item position", error_message);
        });

    c_.device_.clear();
    c_.partition_.clear();
    c_.reference_point_.clear();
    c_.item_name_.clear();
    c_.item_position_ = item_position;

    StrBoUrl::for_each_url_decoded(url.substr(offset, end_of_device - offset),
        [this] (const char ch) { c_.device_ += ch; } ,
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Device", e.c_str()); });
    StrBoUrl::for_each_url_decoded(url.substr(end_of_device + 1,
                                              end_of_partition - end_of_device - 1),
        [this] (const char ch) { c_.partition_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Partition", e.c_str()); });

    if(end_of_partition < end_of_reference)
        StrBoUrl::for_each_url_decoded(url.substr(end_of_partition + 1,
                                                  end_of_reference - end_of_partition - 1),
            [this] (const char ch) { c_.reference_point_ += ch; },
            [] (auto &&e)
            { throw ParsingError(get_error_prefix(), "Reference point", e.c_str()); });

    StrBoUrl::for_each_url_decoded(url.substr(end_of_reference + 1,
                                              end_of_item - end_of_reference - 1),
        [this] (const char ch) { c_.item_name_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), "Item name", e.c_str()); });

    const char *result;

    if(c_.reference_point_ == "/")
    {
        result = "USB location trace contains unneeded explicit reference to root";
        c_.reference_point_.clear();
    }
    else
        result = nullptr;

    is_partition_set_ = true;
    is_item_set_ = true;
    return result;
}
