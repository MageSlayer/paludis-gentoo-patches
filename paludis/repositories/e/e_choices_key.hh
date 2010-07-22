/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_CHOICES_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_CHOICES_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/util/map-fwd.hh>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class ERepositoryID;

        class EChoicesKey :
            public MetadataValueKey<std::shared_ptr<const Choices> >,
            private PrivateImplementationPattern<EChoicesKey>
        {
            private:
                void populate_iuse() const;
                void populate_myoptions() const;

            public:
                EChoicesKey(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &,
                        const std::string &,
                        const MetadataKeyType,
                        const std::shared_ptr<const ERepository> & maybe_profile,
                        const std::shared_ptr<const Map<ChoiceNameWithPrefix, std::string> > & maybe_descriptions);

                ~EChoicesKey();

                const std::shared_ptr<const Choices> value() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
