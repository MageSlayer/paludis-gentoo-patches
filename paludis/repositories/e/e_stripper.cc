/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/e_stripper.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/output_manager.hh>
#include <ostream>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Implementation<EStripper>
    {
        EStripperOptions options;

        Implementation(const EStripperOptions & o) :
            options(o)
        {
        }
    };
}

EStripper::EStripper(const EStripperOptions & options) :
    Stripper(make_named_values<StripperOptions>(
                n::debug_dir() = options.debug_dir(),
                n::image_dir() = options.image_dir(),
                n::split() = options.split(),
                n::strip() = options.strip()
                )),
    PrivateImplementationPattern<EStripper>(options),
    _imp(PrivateImplementationPattern<EStripper>::_imp)
{
}

EStripper::~EStripper()
{
}

void
EStripper::on_strip(const FSEntry & f)
{
    _imp->options.output_manager()->stdout_stream() << "str " << f.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_split(const FSEntry & f, const FSEntry & g)
{
    _imp->options.output_manager()->stdout_stream() << "spl " << f.strip_leading(_imp->options.image_dir()) <<
        " -> " << g.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_unknown(const FSEntry & f)
{
    _imp->options.output_manager()->stdout_stream() << "--- " << f.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_enter_dir(const FSEntry &)
{
}

void
EStripper::on_leave_dir(const FSEntry &)
{
}

void
EStripper::strip()
{
    _imp->options.output_manager()->stdout_stream() << ">>> Stripping inside " << _imp->options.image_dir() << std::endl;
    Stripper::strip();
}

