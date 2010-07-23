/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FORWARD_AT_FINISH_OUTPUT_MANAGER_HH
#define PALUDIS_GUARD_PALUDIS_FORWARD_AT_FINISH_OUTPUT_MANAGER_HH 1

#include <paludis/forward_at_finish_output_manager-fwd.hh>
#include <paludis/output_manager.hh>
#include <paludis/output_manager_factory.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/pimp.hh>
#include <memory>
#include <functional>

namespace paludis
{
    class PALUDIS_VISIBLE ForwardAtFinishOutputManager :
        private Pimp<ForwardAtFinishOutputManager>,
        public OutputManager
    {
        public:
            ForwardAtFinishOutputManager(
                    const bool if_success,
                    const bool if_failure,
                    const std::shared_ptr<OutputManager> & child);
            ~ForwardAtFinishOutputManager();

            virtual std::ostream & stdout_stream() PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual std::ostream & stderr_stream() PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void succeeded();
            virtual void flush();
            virtual bool want_to_flush() const;
            virtual void message(const MessageType, const std::string &);
            virtual void nothing_more_to_come();

            static const std::shared_ptr<const Set<std::string> > factory_managers()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::shared_ptr<OutputManager> factory_create(
                    const OutputManagerFactory::KeyFunction &,
                    const OutputManagerFactory::CreateChildFunction &,
                    const OutputManagerFactory::ReplaceVarsFunc &)
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<ForwardAtFinishOutputManager>;
}

#endif
