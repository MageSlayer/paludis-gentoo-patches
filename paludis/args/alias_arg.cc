/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "alias_arg.hh"
#include "args_group.hh"
#include "args_handler.hh"

/** \file
 * Implementation for AliasArg.
 *
 * \ingroup Args
 */

using namespace paludis::args;

AliasArg::AliasArg(ArgsOption * const other, const std::string & long_name) :
    ArgsOption(other->group(), long_name, '\0', "Alias for --" + other->long_name()),
    _other(other)
{
    other->group()->handler()->add_option(other, long_name);
}

