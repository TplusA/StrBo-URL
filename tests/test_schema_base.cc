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

#include <doctest.h>

#include "strbo_url_schemes.hh"

TEST_SUITE_BEGIN("StrBo URL resource locator base class");

/*!
 * Test fixture: simple resource locator.
 *
 * The code would be *exactly* the same for
 * #StrBoUrl::Schema::ResourceLocatorReference and
 * #StrBoUrl::Schema::TraceLocator, so we only test with
 * #StrBoUrl::Schema::ResourceLocatorSimple.
 */
class SimpleResourceLocatorFixture
{
  public:
    class Locator: public StrBoUrl::Schema::ResourceLocatorSimple
    {
      public:
        explicit Locator():
            StrBoUrl::Schema::ResourceLocatorSimple("testing-simple")
        {}
    };

  protected:
    Locator locator_;
};

TEST_CASE_FIXTURE(SimpleResourceLocatorFixture, "Retrieve scheme name")
{
    CHECK(locator_.get_scheme_name() == "testing-simple");
}

TEST_CASE_FIXTURE(SimpleResourceLocatorFixture, "URL with matching scheme name")
{
    CHECK(locator_.url_matches_scheme("testing-simple://"));
    CHECK(locator_.url_matches_scheme("testing-simple:// "));
    CHECK(locator_.url_matches_scheme("testing-simple://a"));
    CHECK(locator_.url_matches_scheme("testing-simple://hello.world"));
    CHECK(locator_.url_matches_scheme("testing-simple://!@#$%^*():/"));
}

TEST_CASE_FIXTURE(SimpleResourceLocatorFixture, "URL with non-matching scheme name")
{
    CHECK_FALSE(locator_.url_matches_scheme(" testing-simple://"));
    CHECK_FALSE(locator_.url_matches_scheme("esting-simple://"));
    CHECK_FALSE(locator_.url_matches_scheme("testing-simpl://"));
    CHECK_FALSE(locator_.url_matches_scheme("testing-simple:/"));
    CHECK_FALSE(locator_.url_matches_scheme("testing-simple"));
}

TEST_SUITE_END();
