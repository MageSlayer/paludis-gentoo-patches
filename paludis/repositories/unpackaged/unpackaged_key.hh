/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_UNPACKAGED_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        class UnpackagedFSEntryKey :
            public MetadataFSEntryKey,
            private PrivateImplementationPattern<UnpackagedFSEntryKey>
        {
            private:
                Implementation<UnpackagedFSEntryKey> * const _imp;

            public:
                UnpackagedFSEntryKey(const std::string &, const std::string &, const MetadataKeyType, const FSEntry & l);
                ~UnpackagedFSEntryKey();

                const FSEntry value() const;
        };

        class UnpackagedStringKey :
            public MetadataStringKey,
            private PrivateImplementationPattern<UnpackagedStringKey>
        {
            private:
                Implementation<UnpackagedStringKey> * const _imp;

            public:
                UnpackagedStringKey(const std::string & r, const std::string & h, const MetadataKeyType t, const std::string & v);
                ~UnpackagedStringKey();

                const std::string value() const;
        };

        class UnpackagedDependencyKey :
            public MetadataSpecTreeKey<DependencySpecTree>,
            private PrivateImplementationPattern<UnpackagedDependencyKey>
        {
            private:
                Implementation<UnpackagedDependencyKey> * const _imp;

            public:
                UnpackagedDependencyKey(const Environment * const env,
                        const std::string & r, const std::string & h, const MetadataKeyType t,
                        const std::string & v);
                ~UnpackagedDependencyKey();

                const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const;

                std::string pretty_print(const DependencySpecTree::ItemFormatter & f) const;

                std::string pretty_print_flat(const DependencySpecTree::ItemFormatter & f) const;
        };
    }
}

#endif
