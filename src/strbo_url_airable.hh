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

#ifndef STRBO_URL_AIRABLE_HH
#define STRBO_URL_AIRABLE_HH

#include "strbo_url.hh"

#include <vector>

namespace Airable
{

/*!
 * The \c strbo-airable scheme.
 */
class ResourceLocatorSimple: public ::StrBoUrl::Schema::ResourceLocatorSimple
{
  public:
    explicit ResourceLocatorSimple():
        StrBoUrl::Schema::ResourceLocatorSimple("strbo-airable")
    {}
};

/*!
 * The \c strbo-ref-airable scheme.
 */
class ResourceLocatorReference: public ::StrBoUrl::Schema::ResourceLocatorReference
{
  public:
    explicit ResourceLocatorReference():
        StrBoUrl::Schema::ResourceLocatorReference("strbo-ref-airable")
    {}
};

/*!
 * The \c strbo-trace-airable scheme.
 */
class TraceLocator: public ::StrBoUrl::Schema::TraceLocator
{
  public:
    explicit TraceLocator():
        StrBoUrl::Schema::TraceLocator("strbo-trace-airable")
    {}
};

/*!
 * Representation of a Airable simple location key.
 */
class LocationKeySimple: public ::StrBoUrl::Location
{
  public:
    struct Components
    {
        std::string item_url_;

        Components() {}

        explicit Components(std::string &&item_url):
            item_url_(std::move(item_url))
        {}
    };

  private:
    Components c_;
    bool is_item_set_;

  public:
    LocationKeySimple(const LocationKeySimple &) = delete;
    LocationKeySimple &operator=(const LocationKeySimple &) = delete;

    explicit LocationKeySimple():
        ::StrBoUrl::Location(get_scheme()),
        is_item_set_(false)
    {}

    void clear() final override
    {
        c_.item_url_.clear();
        is_item_set_ = false;
    }

    bool is_valid() const final override { return is_item_set_; }

    void set_item(const char *raw_url)
    {
        c_.item_url_ = raw_url;
        is_item_set_ = true;
    }

    void set_item(std::string &&url)
    {
        c_.item_url_ = std::move(url);
        is_item_set_ = true;
    }

    const Components &unpack() const { return c_; }

  protected:
    const char *get_error_prefix_for_exception() const final override { return get_error_prefix(); }
    std::string str_impl() const final override;
    const char *set_url_impl(const std::string &url, size_t offset) final override;

  private:
    static const char *get_error_prefix();

  public:
    static const ::StrBoUrl::Schema::StrBoLocator &get_scheme()
    {
        static const ::Airable::ResourceLocatorSimple scheme;
        return scheme;
    }
};

/*!
 * Representation of a Airable reference location key.
 */
class LocationKeyReference: public ::StrBoUrl::Location
{
  public:
    struct Components
    {
        std::string containing_list_url_;
        std::string item_url_;
        StrBoUrl::ObjectIndex item_position_;

        Components() {}

        explicit Components(std::string &&containing_list_url,
                            std::string &&item_url,
                            StrBoUrl::ObjectIndex item_position):
            containing_list_url_(std::move(containing_list_url)),
            item_url_(std::move(item_url)),
            item_position_(item_position)
        {}
    };

  private:
    Components c_;
    bool is_containing_list_set_;

  public:
    LocationKeyReference(const LocationKeyReference &) = delete;
    LocationKeyReference &operator=(const LocationKeyReference &) = delete;

    explicit LocationKeyReference():
        ::StrBoUrl::Location(get_scheme()),
        is_containing_list_set_(false)
    {}

    void clear() final override
    {
        c_.containing_list_url_.clear();
        c_.item_url_.clear();
        c_.item_position_ = StrBoUrl::ObjectIndex();
        is_containing_list_set_ = false;
    }

    bool is_valid() const final override
    {
        return is_containing_list_set_ && !c_.item_url_.empty() && c_.item_position_.is_valid();
    }

    void set_containing_list(const char *raw_url)
    {
        c_.containing_list_url_ = raw_url;
        is_containing_list_set_ = true;
    }

    void set_containing_list(std::string &&url)
    {
        c_.containing_list_url_ = std::move(url);
        is_containing_list_set_ = true;
    }

    void set_item(const char *raw_url, StrBoUrl::ObjectIndex position)
    {
        c_.item_url_ = raw_url;
        c_.item_position_ = position;
    }

    void set_item(std::string &&url, StrBoUrl::ObjectIndex position)
    {
        c_.item_url_ = std::move(url);
        c_.item_position_ = position;
    }

    const Components &unpack() const { return c_; }

  protected:
    const char *get_error_prefix_for_exception() const final override { return get_error_prefix(); }
    std::string str_impl() const final override;
    const char *set_url_impl(const std::string &url, size_t offset) final override;

  private:
    static const char *get_error_prefix();

  public:
    static const ::StrBoUrl::Schema::StrBoLocator &get_scheme()
    {
        static const ::Airable::ResourceLocatorReference scheme;
        return scheme;
    }
};

/*!
 * Representation of a Airable location trace.
 */
class LocationTrace: public ::StrBoUrl::Location
{
  public:
    struct Components
    {
        std::string reference_point_url_;
        std::vector<std::pair<std::string, StrBoUrl::ObjectIndex>> trace_urls_;
        std::string item_url_;
        StrBoUrl::ObjectIndex item_position_;

        Components() {}

        explicit Components(std::string &&reference_point_url,
                            std::vector<std::pair<std::string, StrBoUrl::ObjectIndex>> &&trace_urls,
                            std::string &&item_url, StrBoUrl::ObjectIndex item_position):
            reference_point_url_(std::move(reference_point_url)),
            trace_urls_(std::move(trace_urls)),
            item_url_(std::move(item_url)),
            item_position_(item_position)
        {}
    };

  private:
    Components c_;
    bool is_reference_point_set_;

  public:
    LocationTrace(const LocationTrace &) = delete;
    LocationTrace &operator=(const LocationTrace &) = delete;

    explicit LocationTrace():
        ::StrBoUrl::Location(get_scheme()),
        is_reference_point_set_(false)
    {}

    void clear() final override
    {
        c_.reference_point_url_.clear();
        c_.trace_urls_.clear();
        c_.item_url_.clear();
        c_.item_position_ = StrBoUrl::ObjectIndex();
        is_reference_point_set_ = false;
    }

    bool is_valid() const final override
    {
        return is_reference_point_set_ && !c_.item_url_.empty() && c_.item_position_.is_valid();
    }

    size_t get_trace_length() const;

    void set_reference_point(const char *raw_url)
    {
        c_.reference_point_url_ = raw_url;
        is_reference_point_set_ = true;
    }

    void set_reference_point(std::string &&url)
    {
        c_.reference_point_url_ = std::move(url);
        is_reference_point_set_ = true;
    }

    void append_to_trace(const char *raw_url, StrBoUrl::ObjectIndex position)
    {
        c_.trace_urls_.emplace_back(std::make_pair(std::string(raw_url), position));
    }

    void append_to_trace(std::string &&raw_url, StrBoUrl::ObjectIndex position)
    {
        c_.trace_urls_.emplace_back(std::make_pair(std::move(raw_url), position));
    }

    void set_item(const char *raw_url, StrBoUrl::ObjectIndex position)
    {
        c_.item_url_ = raw_url;
        c_.item_position_ = position;
    }

    void set_item(std::string &&url, StrBoUrl::ObjectIndex position)
    {
        c_.item_url_ = std::move(url);
        c_.item_position_ = position;
    }

    const Components &unpack() const { return c_; }

  protected:
    const char *get_error_prefix_for_exception() const final override { return get_error_prefix(); }
    std::string str_impl() const final override;
    const char *set_url_impl(const std::string &url, size_t offset) final override;

  private:
    static const char *get_error_prefix();

  public:
    static const ::StrBoUrl::Schema::StrBoLocator &get_scheme()
    {
        static const ::Airable::TraceLocator scheme;
        return scheme;
    }
};

}

#endif /* !STRBO_URL_AIRABLE_HH */
