/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_KEYWORDS_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_KEYWORDS_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/singleton.hh>
#include <paludis/repositories/e/eapi-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class EKeywordsKeyStore :
            public Singleton<EKeywordsKeyStore>
        {
            friend class Singleton<EKeywordsKeyStore>;

            private:
                Pimp<EKeywordsKeyStore> _imp;

                EKeywordsKeyStore();
                ~EKeywordsKeyStore();

            public:
                const std::shared_ptr<const MetadataCollectionKey<Set<KeywordName> > > fetch(
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &,
                        const MetadataKeyType) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

    }
}

#endif
