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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_OUTPUT_CONF_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_OUTPUT_CONF_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <memory>

namespace paludis
{
    class PaludisEnvironment;

    namespace paludis_environment
    {
        class OutputConf :
            private Pimp<OutputConf>
        {
            public:
                ///\name Basic operations
                ///\{

                OutputConf(const PaludisEnvironment * const);
                ~OutputConf();

                OutputConf(const OutputConf &) = delete;
                OutputConf & operator= (const OutputConf &) = delete;

                ///\}

                /**
                 * Add another file.
                 */
                void add(const FSEntry &);

                const std::shared_ptr<OutputManager> create_output_manager(
                        const CreateOutputManagerInfo &) const;

                const std::shared_ptr<OutputManager> create_named_output_manager(
                        const std::string & s, const CreateOutputManagerInfo & n) const;
        };
    }

    extern template class Pimp<paludis_environment::OutputConf>;
}

#endif
