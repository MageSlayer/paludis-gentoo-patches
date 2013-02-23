/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Saleem Abdulrasool <compnerd@compnerd.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/name.hh>
#include <paludis/partitioning.hh>
#include <paludis/util/fs_path.hh>

#include <gtest/gtest.h>

#include <memory>

using namespace paludis;

namespace
{
    const FSPath kBin("/bin");
    const FSPath kLib("/lib");
    const FSPath kTest("test");

    const PartName kBinaries("binaries");
    const PartName kCore("");
}

TEST(Partitioning, DefaultParts)
{
    auto parts(std::make_shared<Partitioning>());

    parts->mark(std::vector<FSPath>{ kBin }, kBinaries);

    ASSERT_EQ(kBinaries, parts->classify(kBin));
    ASSERT_EQ(kCore, parts->classify(kLib));
}

TEST(Partitioning, PartsExludes)
{
    auto parts(std::make_shared<Partitioning>());

    parts->mark(std::vector<FSPath>{ kBin }, kBinaries);
    parts->mark(std::vector<FSPath>{ kBin / kTest }, kCore);

    ASSERT_EQ(kBinaries, parts->classify(kBin));
    ASSERT_EQ(kCore, parts->classify(kBin / kTest));
}

TEST(Partitioning, PartsOrder)
{
    auto parts(std::make_shared<Partitioning>());

    parts->mark(std::vector<FSPath>{ kBin / kTest }, kCore);
    parts->mark(std::vector<FSPath>{ kBin }, kBinaries);

    ASSERT_EQ(kBinaries, parts->classify(kBin));
    ASSERT_EQ(kBinaries, parts->classify(kBin / kTest));
}

TEST(Partitioning, NameValidation)
{
    ASSERT_THROW(PartName(" "), PartNameError);
    ASSERT_THROW(PartName(","), PartNameError);
}

