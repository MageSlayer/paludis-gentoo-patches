/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/unpackaged/unpackaged_stripper.hh>
#include <paludis/util/kc.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <iostream>

using namespace paludis;
using namespace paludis::unpackaged_repositories;

namespace paludis
{
    template <>
    struct Implementation<UnpackagedStripper>
    {
        UnpackagedStripperOptions options;

        Implementation(const UnpackagedStripperOptions & o) :
            options(o)
        {
        }
    };
}

UnpackagedStripper::UnpackagedStripper(const UnpackagedStripperOptions & options) :
    Stripper(StripperOptions::named_create()
            (k::image_dir(), options[k::image_dir()])
            (k::debug_build(), options[k::debug_build()])
            (k::debug_dir(), options[k::debug_dir()])
            ),
    PrivateImplementationPattern<UnpackagedStripper>(new Implementation<UnpackagedStripper>(options)),
    _imp(PrivateImplementationPattern<UnpackagedStripper>::_imp)
{
}

UnpackagedStripper::~UnpackagedStripper()
{
}

void
UnpackagedStripper::on_strip(const FSEntry & f)
{
    std::cout << "str " << f.strip_leading(_imp->options[k::image_dir()]) << std::endl;
}

void
UnpackagedStripper::on_split(const FSEntry & f, const FSEntry & g)
{
    std::cout << "spl " << f.strip_leading(_imp->options[k::image_dir()]) <<
        " -> " << g.strip_leading(_imp->options[k::image_dir()]) << std::endl;
}

void
UnpackagedStripper::on_unknown(const FSEntry & f)
{
    std::cout << "--- " << f.strip_leading(_imp->options[k::image_dir()]) << std::endl;
}

void
UnpackagedStripper::on_enter_dir(const FSEntry &)
{
}

void
UnpackagedStripper::on_leave_dir(const FSEntry &)
{
}

void
UnpackagedStripper::strip()
{
    std::cout << ">>> Stripping inside " << _imp->options[k::image_dir()] << std::endl;
    Stripper::strip();
}

