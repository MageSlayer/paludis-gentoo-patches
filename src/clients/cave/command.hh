/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/args/man.hh>
#include <paludis/environment-fwd.hh>
#include <memory>

namespace paludis
{
    namespace cave
    {
        enum CommandImportance
        {
            ci_core,
            ci_supplemental,
            ci_development,
            ci_scripting,
            ci_internal,
            ci_ignore,
            last_ci
        };

        class PALUDIS_VISIBLE Command
        {
            public:
                virtual ~Command() = 0;

                virtual CommandImportance importance() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual int run(
                        const std::shared_ptr<Environment> &,
                        const std::shared_ptr<const Sequence<std::string > > & args
                        ) PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

                virtual std::shared_ptr<args::ArgsHandler> make_doc_cmdline() = 0;
        };
    }
}

#endif
