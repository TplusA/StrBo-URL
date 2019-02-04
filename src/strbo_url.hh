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

#ifndef STRBO_URL_HH
#define STRBO_URL_HH

#include "strbo_url_schemes.hh"

#include <exception>

namespace StrBoUrl
{

/*!
 * Base class for Streaming Board location URLs.
 *
 * A location URL can be a location key (following a resource locator scheme)
 * or a location trace (folowing a trace locator scheme).
 */
class Location
{
  public:
    class WrongSchemeError: public std::exception {};

    class ParsingError: public std::exception
    {
      public:
        const std::string error_prefix_;
        const std::string component_name_;
        const std::string error_message_;
        mutable std::string what_buffer;

        explicit ParsingError(const char *error_prefix, const char *component_name,
                              const char *error_message):
            error_prefix_(error_prefix),
            component_name_(component_name != nullptr ? component_name : "URL"),
            error_message_(error_message)
        {}

        const char *what() const throw() override
        {
            if(what_buffer.empty())
            {
                what_buffer = error_prefix_;
                what_buffer += error_message_;

                if(!component_name_.empty())
                    what_buffer += " [" + component_name_ + ']';
            }

            return what_buffer.c_str();
        }
    };

    class InvalidCharactersError: public ParsingError
    {
      public:
        explicit InvalidCharactersError(const char *error_prefix):
            ParsingError(error_prefix, nullptr, "Invalid characters in URL")
        {}
    };

    static const std::string valid_characters;
    static const std::string safe_characters;

  protected:
    const Schema::StrBoLocator &scheme_;

    explicit Location(const Schema::StrBoLocator &scheme):
        scheme_(scheme)
    {}

  public:
    Location(const Location &) = delete;
    Location &operator=(const Location &) = delete;

    virtual ~Location() {}

    virtual void clear() = 0;
    virtual bool is_valid() const = 0;

    std::string str() const
    {
        if(is_valid())
            return str_impl();
        else
            return "";
    }

    /*
     * Set URL object from raw string.
     *
     * In case the URL scheme doesn't match the expected scheme, a
     * #StrBoUrl::Location::WrongSchemeError exception will be thrown.
     *
     * In case of any parsing errors, a #StrBoUrl::Location::ParsingError
     * exception will be thrown.
     *
     * This function usually returns \c nullptr, but when it doesn't, its
     * return value points to a static warning string.
     */
    const char *set_url(const std::string &url)
    {
        if(!scheme_.url_matches_scheme(url))
            throw WrongSchemeError();

        if(url.find_first_not_of(valid_characters) != std::string::npos)
            throw InvalidCharactersError(get_error_prefix_for_exception());

        return set_url_impl(url, scheme_.get_scheme_name().length() + 3);
    }

  protected:
    /*!
     * Return string containing a location-specific error prefix.
     */
    virtual const char *get_error_prefix_for_exception() const = 0;

    /*!
     * Return string representation of the location.
     *
     * This function is supposed to return a URL following its configured
     * scheme.
     *
     * Contract: This function is called only if a preceding call of
     *     #StrBoUrl::Location::is_valid() returned \c true. Therefore, this
     *     function needs to perform no further checks and is required to
     *     return a valid, non-empty URL.
     */
    virtual std::string str_impl() const = 0;

    /*!
     * Set URL object by string.
     *
     * This function is supposed to parse the URL and initialize the object
     * using the URL components.
     *
     * In case of any parsing errors, this function is supposed to throw a
     * #StrBoUrl::Location::ParsingError exception.
     *
     * It shall not simply copy the URL. The #StrBoUrl::Location::str()
     * function member shall always generate URL strings from the object state,
     * not from some copied string.
     *
     * Contract: The URL scheme is guaranteed to match the configurated scheme.
     *     Implementations should not check the scheme prefix again. The offset
     *     parameter points at the first character after the scheme definition.
     */
    virtual const char *set_url_impl(const std::string &url, size_t offset) = 0;
};

/*!
 * Expected position of an object in its parent container, starting at 1.
 *
 * The resource referenced by a reference scheme or a trace locator scheme is
 * supposed to be found in a list. The position of that resource, or object, in
 * that list is specified using the #StrBoUrl::ObjectIndex type.
 *
 * For fast reconstruction of traces, each component on the path from the
 * reference point to the resource is annotated with such an index.
 */
class ObjectIndex
{
  private:
    uint32_t idx_;

  public:
    constexpr explicit ObjectIndex(uint32_t idx = 0):
        idx_(idx)
    {}

    bool is_valid() const { return idx_ > 0; }

    uint32_t get_object_index() const { return idx_; }
};

}

#endif /* !STRBO_URL_HH */
