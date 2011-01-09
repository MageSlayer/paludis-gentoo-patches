/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_FACTORY_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_COMMAND_FACTORY_HH 1

#include <paludis/util/singleton.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <string>
#include <memory>
#include "command.hh"

namespace paludis
{
    namespace cave
    {
        class PALUDIS_VISIBLE UnknownCommand :
            public Exception
        {
            public:
                UnknownCommand(const std::string &) throw ();
        };

        class PALUDIS_VISIBLE CommandFactory :
            public Singleton<CommandFactory>
        {
            friend class Singleton<CommandFactory>;

            private:
                Pimp<CommandFactory> _imp;

                CommandFactory();
                ~CommandFactory();

            public:
                const std::shared_ptr<Command> create(const std::string &)
                    const PALUDIS_ATTRIBUTE((warn_unused_result));

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Singleton<cave::CommandFactory>;
    extern template class Pimp<cave::CommandFactory>;
}

#endif
