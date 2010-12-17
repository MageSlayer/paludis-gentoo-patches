/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_SUGGESTIONS_CONF_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_SUGGESTIONS_CONF_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/tribool.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/create_output_manager_info-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <memory>

namespace paludis
{
    class PaludisEnvironment;

    namespace paludis_environment
    {
        class SuggestionsConf :
            private Pimp<SuggestionsConf>
        {
            public:
                ///\name Basic operations
                ///\{

                SuggestionsConf(const PaludisEnvironment * const);
                ~SuggestionsConf();

                SuggestionsConf(const SuggestionsConf &) = delete;
                SuggestionsConf & operator= (const SuggestionsConf &) = delete;

                ///\}

                /**
                 * Add another file.
                 */
                void add(const FSPath &);

                Tribool interest_in_suggestion(
                        const std::shared_ptr<const PackageID> & from_id,
                        const PackageDepSpec & spec) const;
        };
    }

    extern template class Pimp<paludis_environment::SuggestionsConf>;
}

#endif
