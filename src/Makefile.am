#
# Copyright (C) 2023  T+A elektroakustik GmbH & Co. KG
#
# This file is part of T+A StrBo-URL.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#

ACLOCAL_AMFLAGS = -I ../m4

AM_CFLAGS = $(CWARNINGS)
AM_CXXFLAGS = $(CXXWARNINGS)

noinst_LTLIBRARIES = libstrbo_url.la

libstrbo_url_la_SOURCES = \
    strbo_url.cc strbo_url.hh strbo_url_schemes.hh strbo_url_helpers.hh \
    strbo_url_airable.cc strbo_url_airable.hh \
    strbo_url_upnp.cc strbo_url_upnp.hh \
    strbo_url_usb.cc strbo_url_usb.hh
libstrbo_url_la_CFLAGS = $(AM_CFLAGS)
libstrbo_url_la_CXXFLAGS = $(AM_CXXFLAGS)
