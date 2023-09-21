/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/output_manager.hh>
#include <paludis/package_id.hh>
#include <ostream>

using namespace paludis;
using namespace paludis::erepository;

namespace paludis
{
    template <>
    struct Imp<EStripper>
    {
        EStripperOptions options;

        Imp(const EStripperOptions & o) :
            options(o)
        {
        }
    };
}

EStripper::EStripper(const EStripperOptions & options) :
    Stripper(make_named_values<StripperOptions>(
                n::compress_splits() = options.compress_splits(),
                n::controllable_strip() = static_cast<const ERepositoryID &>(*options.package_id()).eapi()->supported()->tools_options()->controllable_strip(),
                n::controllable_strip_dir() = options.controllable_strip_dir(),
                n::debug_dir() = options.debug_dir(),
                n::dwarf_compression() = options.dwarf_compression(),
                n::image_dir() = options.image_dir(),
                n::split() = options.split(),
                n::strip_choice() = options.strip_choice(),
                n::strip_restrict() = options.strip_restrict()
                )),
    _imp(options)
{
}

EStripper::~EStripper() = default;

void
EStripper::on_enter_dir(const FSPath &)
{
}

void
EStripper::on_leave_dir(const FSPath &)
{
}

void
EStripper::on_strip(const FSPath & f)
{
    _imp->options.output_manager()->stdout_stream()
        << strip_action_desc() << " "
        << f.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_split(const FSPath & f, const FSPath & g)
{
    _imp->options.output_manager()->stdout_stream()
        << split_action_desc() << " "
        << f.strip_leading(_imp->options.image_dir())
        << " -> " << g.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_dwarf_compress(const FSPath & f)
{
    _imp->options.output_manager()->stdout_stream()
        << dwarf_compress_desc() << " "
        << f.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::on_unknown(const FSPath & f)
{
    _imp->options.output_manager()->stdout_stream()
        << unknown_action_desc() << " "
        << f.strip_leading(_imp->options.image_dir()) << std::endl;
}

void
EStripper::strip()
{
    _imp->options.output_manager()->stdout_stream()
        << ">>> Stripping inside " << _imp->options.image_dir() << std::endl;
    Stripper::strip();
}

