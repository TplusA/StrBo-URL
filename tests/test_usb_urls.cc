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

#include "strbo_url_usb.hh"

TEST_SUITE_BEGIN("StrBo USB URLs");

class SimpleLocatorFixture
{
  protected:
    USB::LocationKeySimple url;
};

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Locator scheme is as expected")
{
    CHECK(url.get_scheme().get_scheme_name() == "strbo-usb");
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Empty locator is invalid")
{
    CHECK_FALSE(url.is_valid());
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Representation of invalid locator is empty string")
{
    CHECK(url.str().empty());
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Set locator from URL with wrong scheme throws")
{
    CHECK_THROWS_AS(url.set_url("strbo-us://not_parsed"),
                    StrBoUrl::Location::WrongSchemeError);
    CHECK(url.str().empty());
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Set locator for file from valid USB URL string")
{
    const std::string expected("strbo-usb://usb-Generic_Flash_Disk_1EB86759-0%3A0:usb-Generic_Flash_Disk_1EB86759-0%3A0-part1/Music%2FSome%20Album%2F05%20-%20Song.flac");
    CHECK(url.set_url(expected) == nullptr);
    REQUIRE(url.is_valid());
    CHECK(url.str() == expected);

    const auto &c(url.unpack());
    CHECK(c.device_ == "usb-Generic_Flash_Disk_1EB86759-0:0");
    CHECK(c.partition_ == "usb-Generic_Flash_Disk_1EB86759-0:0-part1");
    CHECK(c.path_ == "Music/Some Album/05 - Song.flac");
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Set locator for partition ID from valid USB URL string")
{
    const std::string expected("strbo-usb://Flash_Disk:usb-Generic_Flash_Disk_1EB86759-0%3A0-part1/");
    CHECK(url.set_url(expected) == nullptr);
    REQUIRE(url.is_valid());
    CHECK(url.str() == expected);

    const auto &c(url.unpack());
    CHECK(c.device_ == "Flash_Disk");
    CHECK(c.partition_ == "usb-Generic_Flash_Disk_1EB86759-0:0-part1");
    CHECK(c.path_.empty());
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Set locator for file by componenents")
{
    url.set_device("My USB Device");
    url.set_partition("usb-Generic_Flash_Disk_1EB86759-0:0-part1");
    CHECK_FALSE(url.is_valid());
    url.set_path("Music/Some Album/05 - Song.flac");
    REQUIRE(url.is_valid());

    const std::string expected("strbo-usb://My%20USB%20Device:usb-Generic_Flash_Disk_1EB86759-0%3A0-part1/Music%2FSome%20Album%2F05%20-%20Song.flac");
    CHECK(url.str() == expected);
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Set locator for file by appending")
{
    url.set_device("My USB Device");
    url.set_partition("usb-Generic_Flash_Disk_1EB86759-0:0-part1");
    CHECK_FALSE(url.is_valid());
    url.append_to_path("Music");
    CHECK(url.is_valid());
    url.append_to_path("Some Album");
    url.append_to_path("05 - Song.flac");
    REQUIRE(url.is_valid());

    const std::string expected("strbo-usb://My%20USB%20Device:usb-Generic_Flash_Disk_1EB86759-0%3A0-part1/Music%2FSome%20Album%2F05%20-%20Song.flac");
    CHECK(url.str() == expected);
}

TEST_CASE_FIXTURE(SimpleLocatorFixture, "Clear locator")
{
    CHECK(url.set_url("strbo-usb://dev:part/file") == nullptr);
    REQUIRE(url.is_valid());
    url.clear();
    CHECK_FALSE(url.is_valid());
}

TEST_SUITE_END();
