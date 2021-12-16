/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_TEE_OUTPUT_MANAGER_HH
#define PALUDIS_GUARD_PALUDIS_TEE_OUTPUT_MANAGER_HH 1

#include <paludis/tee_output_manager-fwd.hh>
#include <paludis/output_manager.hh>
#include <paludis/output_manager_factory.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/pimp.hh>
#include <memory>
#include <functional>

namespace paludis
{
    class PALUDIS_VISIBLE TeeOutputManager :
        public OutputManager
    {
        private:
            Pimp<TeeOutputManager> _imp;

        public:
            TeeOutputManager(
                    const std::shared_ptr<const OutputManagerSequence> &,
                    const std::shared_ptr<const OutputManagerSequence> &) PALUDIS_ATTRIBUTE((deprecated));

            TeeOutputManager(
                    const std::shared_ptr<const OutputManagerSequence> &,
                    const std::shared_ptr<const OutputManagerSequence> &,
                    const std::shared_ptr<const OutputManagerSequence> &,
                    const std::shared_ptr<const OutputManagerSequence> &);

            ~TeeOutputManager() override;

            std::ostream & stdout_stream() override PALUDIS_ATTRIBUTE((warn_unused_result));
            std::ostream & stderr_stream() override PALUDIS_ATTRIBUTE((warn_unused_result));

            void succeeded() override;
            void ignore_succeeded() override;
            void flush() override;
            bool want_to_flush() const override;
            void nothing_more_to_come() override;
            void message(const MessageType, const std::string &) override;

            static const std::shared_ptr<const Set<std::string> > factory_managers()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            static const std::shared_ptr<OutputManager> factory_create(
                    const OutputManagerFactory::KeyFunction &,
                    const OutputManagerFactory::CreateChildFunction &,
                    const OutputManagerFactory::ReplaceVarsFunc &)
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Pimp<TeeOutputManager>;
}

#endif
