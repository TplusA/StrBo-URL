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

compiler = meson.get_compiler('cpp')

if not compiler.has_header('doctest.h')
    subdir_done()
endif

testrunner_lib = static_library('testrunner', 'testrunner.cc',
    cpp_args: '-DDOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING',
    include_directories: '../src'
)

test('Schema base tests',
    executable('test_schema_base',
        'test_schema_base.cc',
        include_directories: '../src',
        link_with: testrunner_lib,
        build_by_default: false
    ),
    workdir: meson.current_build_dir(),
    args: ['--reporters=strboxml', '--out=test_schema_base.junit.xml']
)

test('USB URL schemas',
    executable('test_usb_urls',
        'test_usb_urls.cc',
        include_directories: '../src',
        link_with: [testrunner_lib, strbo_url_lib],
        build_by_default: false
    ),
    workdir: meson.current_build_dir(),
    args: ['--reporters=strboxml', '--out=test_schema_base.junit.xml']
)
