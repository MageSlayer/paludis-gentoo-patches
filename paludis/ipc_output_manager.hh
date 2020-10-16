/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_IPC_OUTPUT_MANAGER_HH
#define PALUDIS_GUARD_PALUDIS_IPC_OUTPUT_MANAGER_HH 1

#include <paludis/ipc_output_manager-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/output_manager.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/action-fwd.hh>
#include <memory>
#include <functional>
#include <string>

namespace paludis
{
    class PALUDIS_VISIBLE IPCOutputManager :
        public OutputManager
    {
        private:
            Pimp<IPCOutputManager> _imp;

        public:
            IPCOutputManager(
                    const int pipe_read_fd,
                    const int pipe_write_fd,
                    const CreateOutputManagerInfo &);
            ~IPCOutputManager() noexcept(false) override;

            std::ostream & stdout_stream() override PALUDIS_ATTRIBUTE((warn_unused_result));
            std::ostream & stderr_stream() override PALUDIS_ATTRIBUTE((warn_unused_result));

            void succeeded() override;
            void ignore_succeeded() override;
            void flush() override;
            bool want_to_flush() const override;
            void nothing_more_to_come() override;
            void message(const MessageType, const std::string &) override;
    };

    class PALUDIS_VISIBLE IPCInputManager
    {
        private:
            Pimp<IPCInputManager> _imp;

            std::string _pipe_command_handler(const std::string &);
            void _copy_thread();

        public:
            IPCInputManager(
                    const Environment * const,
                    const std::function<void (const std::shared_ptr<OutputManager> &)> &);

            ~IPCInputManager() noexcept(false);

            const std::function<std::string (const std::string &)> pipe_command_handler()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * The underlying output manager.
             *
             * The underlying output manager is not constructed until the child
             * process requests it, so a null pointer may be returned.
             *
             * Normally the output manager is destroyed when the
             * IPCInputManager is destroyed, but keeping the shared pointer
             * this method returns around for longer will also work, if, for
             * example, any 'finished' messages are to be displayed later on.
             */
            const std::shared_ptr<OutputManager> underlying_output_manager_if_constructed() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE OutputManagerFromIPC
    {
        private:
            Pimp<OutputManagerFromIPC> _imp;

        public:
            OutputManagerFromIPC(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &,
                    const OutputExclusivity,
                    const ClientOutputFeatures &
                    );

            ~OutputManagerFromIPC() noexcept(false);

            const std::shared_ptr<OutputManager> operator() (const Action &);

            const std::shared_ptr<OutputManager> output_manager_if_constructed();

            void construct_standard_if_unconstructed();
    };

    extern template class Pimp<IPCOutputManager>;
    extern template class Pimp<IPCInputManager>;
    extern template class Pimp<OutputManagerFromIPC>;

}

#endif
