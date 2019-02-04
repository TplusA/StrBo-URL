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

#include "strbo_url_airable.hh"
#include "strbo_url_helpers.hh"

#include <sstream>

std::string Airable::LocationKeySimple::str_impl() const
{
    std::string result = scheme_.get_scheme_name() + "://";

    StrBoUrl::for_each_url_encoded(c_.item_url_,
        [&result] (const char *enc, size_t len) { result.append(enc, len); });

    return result;
}

const char *Airable::LocationKeySimple::get_error_prefix()
{
    return "Simple Airable location key malformed: ";
}

const char *Airable::LocationKeySimple::set_url_impl(const std::string &url,
                                                     size_t offset)
{
    c_.item_url_.clear();
    is_item_set_ = true;

    if(offset >= url.length())
        return "Simple Airable location key is empty";

    StrBoUrl::for_each_url_decoded(url.substr(offset),
        [this] (const char ch) { c_.item_url_ += ch; },
        [] (auto &&e)
        { throw ParsingError(get_error_prefix(), nullptr, e.c_str()); } );

    const char *result;

    if(c_.item_url_ == "/")
    {
        result = "Simple Airable location key contains unneeded explicit reference to root";
        c_.item_url_.clear();
    }
    else
        result = nullptr;

    return result;
}

std::string Airable::LocationKeyReference::str_impl() const
{
    std::ostringstream os;
    os << scheme_.get_scheme_name() << "://";

    StrBoUrl::for_each_url_encoded(c_.containing_list_url_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << '/';

    StrBoUrl::for_each_url_encoded(c_.item_url_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':' << c_.item_position_.get_object_index();

    return os.str();
}

const char *Airable::LocationKeyReference::get_error_prefix()
{
    return "Reference Airable location key malformed: ";
}

const char *Airable::LocationKeyReference::set_url_impl(const std::string &url,
                                                        size_t offset)
{
    const auto end_of_reference = StrBoUrl::Parse::extract_field(
        url, offset, '/', StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Reference point", error_message);
        });

    const auto end_of_item = StrBoUrl::Parse::extract_field(
        url, end_of_reference + 1, ':',
        StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item", error_message);
        });

    const auto item_position = StrBoUrl::Parse::item_position(
        url, end_of_item + 1,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item position", error_message);
        });

    c_.containing_list_url_.clear();
    c_.item_url_.clear();
    c_.item_position_ = item_position;
    is_containing_list_set_ = true;

    if(offset < end_of_reference)
        StrBoUrl::for_each_url_decoded(url.substr(offset, end_of_reference - offset),
            [this] (const char ch) { c_.containing_list_url_ += ch; },
            [] (std::string &&e) { throw ParsingError(get_error_prefix(), nullptr, e.c_str()); } );

    StrBoUrl::for_each_url_decoded(url.substr(end_of_reference + 1,
                                              end_of_item - end_of_reference - 1),
        [this] (const char ch) { c_.item_url_ += ch; },
        [] (std::string &&e) { throw ParsingError(get_error_prefix(), nullptr, e.c_str()); } );

    const char *result;

    if(c_.containing_list_url_ == "/")
    {
        result = "Reference Airable location key contains unneeded explicit reference to root";

        c_.containing_list_url_.clear();
    }
    else
        result = nullptr;

    return result;
}

std::string Airable::LocationTrace::str_impl() const
{
    std::ostringstream os;
    os << scheme_.get_scheme_name() << "://";

    StrBoUrl::for_each_url_encoded(c_.reference_point_url_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    if(!c_.trace_urls_.empty())
        os << '/';

    bool is_first = true;

    for(const auto &component : c_.trace_urls_)
    {
        if(is_first)
            is_first = false;
        else
            os << ':';

        StrBoUrl::for_each_url_encoded(component.first,
            [&os] (const char *enc, size_t len) { os.write(enc, len); });

        os << ':' << component.second.get_object_index();
    }

    os << '/';

    StrBoUrl::for_each_url_encoded(c_.item_url_,
        [&os] (const char *enc, size_t len) { os.write(enc, len); });

    os << ':' << c_.item_position_.get_object_index();

    return os.str();
}

static void parse_trace(const std::string &t,
                        const size_t start, const size_t end,
                        decltype(Airable::LocationTrace::Components::trace_urls_) &trace,
                        const std::function<void(const char *component_name, const char *error_message)> &on_error)
{
    if(start >= end)
    {
        on_error(nullptr, "Empty trace");
        return;
    }

    size_t start_of_token = start;
    bool expecting_item_url = true;

    while(start_of_token < end)
    {
        auto end_of_field = t.find(':', start_of_token);

        if(end_of_field == std::string::npos)
            end_of_field = end;
        else if(end_of_field == start_of_token)
        {
            on_error(nullptr, "Empty field in trace");
            return;
        }

        if(expecting_item_url)
        {
            trace.emplace_back(std::make_pair(std::string(), StrBoUrl::ObjectIndex()));
            auto &dest(trace.back().first);
            StrBoUrl::for_each_url_decoded(t.substr(start_of_token, end_of_field - start_of_token),
                [&dest] (const char ch) { dest += ch; },
                [&on_error] (std::string &&e) { on_error("Trace item URL", e.c_str()); } );
        }
        else
        {
            trace.back().second = StrBoUrl::Parse::item_position(
                t, start_of_token, end_of_field,
                [&on_error] (const char *error_message)
                {
                    on_error("Trace item position", error_message);
                });

            if(!trace.back().second.is_valid())
                return;
        }

        expecting_item_url = !expecting_item_url;
        start_of_token = end_of_field + 1;
    }

    if(!expecting_item_url)
        on_error(nullptr, "Odd number of fields in trace");
}

const char *Airable::LocationTrace::get_error_prefix()
{
    return "Airable location trace malformed: ";
}

const char *Airable::LocationTrace::set_url_impl(const std::string &url, size_t offset)
{
    const auto end_of_reference = StrBoUrl::Parse::extract_field(
        url, offset, '/', StrBoUrl::Parse::FieldPolicy::MAY_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Reference point", error_message);
        });

    const auto end_of_trace = StrBoUrl::Parse::extract_field(
        url, end_of_reference + 1, '/', StrBoUrl::Parse::FieldPolicy::FIELD_OPTIONAL,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Trace", error_message);
        });

    const size_t start_of_item = (end_of_trace == std::string::npos
                                  ? end_of_reference
                                  : end_of_trace) + 1;
    const bool is_trace_empty =
        end_of_trace == std::string::npos || end_of_trace == end_of_reference;

    const auto end_of_item = StrBoUrl::Parse::extract_field(
        url, start_of_item, ':', StrBoUrl::Parse::FieldPolicy::MUST_NOT_BE_EMPTY,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item", error_message);
        });

    const auto item_position = StrBoUrl::Parse::item_position(
        url, end_of_item + 1,
        [] (const char *error_message)
        {
            throw ParsingError(get_error_prefix(), "Item position", error_message);
        });

    decltype(Components::trace_urls_) trace;

    if(!is_trace_empty)
       parse_trace(url, end_of_reference + 1, end_of_trace, trace,
                   [] (const char *component_name, const char *error_message)
                   {
                       throw ParsingError(get_error_prefix(), component_name, error_message);
                   });

    c_.reference_point_url_.clear();
    c_.trace_urls_ = std::move(trace);
    c_.item_url_.clear();
    c_.item_position_ = item_position;
    is_reference_point_set_ = true;

    if(offset < end_of_reference)
        StrBoUrl::for_each_url_decoded(url.substr(offset, end_of_reference - offset),
            [this] (const char ch) { c_.reference_point_url_ += ch; },
            [] (std::string &&e) { throw ParsingError(get_error_prefix(), "Reference point URL", e.c_str()); } );

    StrBoUrl::for_each_url_decoded(url.substr(start_of_item, end_of_item - start_of_item),
        [this] (const char ch) { c_.item_url_ += ch; },
        [] (std::string &&e) { throw ParsingError(get_error_prefix(), "Reference item URL", e.c_str()); } );

    const char *result;

    if(c_.reference_point_url_ == "/")
    {
        result = "Airable location trace contains unneeded explicit reference to root";
        c_.reference_point_url_.clear();
    }
    else
        result = nullptr;

    return result;
}
