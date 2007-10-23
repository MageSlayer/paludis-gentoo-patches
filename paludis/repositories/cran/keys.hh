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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_CRAN_KEYS_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_CRAN_KEYS_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/sequence.hh>

namespace paludis
{
    namespace cranrepository
    {
        class CRANPackageID;

        class SimpleURIKey :
            public MetadataSpecTreeKey<SimpleURISpecTree>
        {
            private:
                const std::string _v;

            public:
                SimpleURIKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const SimpleURISpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const SimpleURISpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class StringKey :
            public MetadataStringKey
        {
            private:
                const std::string _v;

            public:
                StringKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType);

                virtual const std::string value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class FSLocationKey :
            public MetadataFSEntryKey
        {
            private:
                const FSEntry _v;

            public:
                FSLocationKey(const std::string &, const std::string &, const FSEntry &, const MetadataKeyType);

                virtual const FSEntry value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PackageIDSequenceKey :
            public MetadataSetKey<PackageIDSequence>
        {
            private:
                const Environment * const _env;
                const tr1::shared_ptr<PackageIDSequence> _v;

            public:
                PackageIDSequenceKey(const Environment * const,
                        const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const PackageIDSequence> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void push_back(const tr1::shared_ptr<const PackageID> &);

                virtual std::string pretty_print_flat(const Formatter<tr1::shared_ptr<const PackageID> > &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PackageIDKey :
            public MetadataPackageIDKey
        {
            private:
                const CRANPackageID * const _v;

            public:
                PackageIDKey(const std::string &, const std::string &, const CRANPackageID * const, const MetadataKeyType);

                virtual const tr1::shared_ptr<const PackageID> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class DepKey :
            public MetadataSpecTreeKey<DependencySpecTree>
        {
            private:
                const Environment * const _env;
                mutable Mutex _m;
                mutable tr1::shared_ptr<DependencySpecTree::ConstItem> _c;
                const std::string _v;

            public:
                DepKey(const Environment * const,
                        const std::string &, const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
