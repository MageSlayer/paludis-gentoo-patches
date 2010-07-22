/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_COMMA_SEPARATED_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_COMMA_SEPARATED_DEP_PARSER_HH 1

#include <paludis/spec_tree-fwd.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE CommaSeparatedDepParser
    {
        public:
            CommaSeparatedDepParser() = delete;

            static std::shared_ptr<const DependencySpecTree> parse(
                    const Environment * const env, const std::string &);
    };
}

#endif
