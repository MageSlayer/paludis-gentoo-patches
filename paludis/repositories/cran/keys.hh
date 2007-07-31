/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/mutex.hh>

namespace paludis
{
    namespace cranrepository
    {
        class CRANPackageID;

        class URIKey :
            public MetadataSpecTreeKey<URISpecTree>
        {
            private:
                const std::string _v;

            public:
                URIKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const URISpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat() const
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

        class PackageIDSequenceKey :
            public MetadataSetKey<PackageIDSequence>
        {
            private:
                const tr1::shared_ptr<PackageIDSequence> _v;

            public:
                PackageIDSequenceKey(const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const PackageIDSequence> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void push_back(const tr1::shared_ptr<const PackageID> &);
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
                mutable Mutex _m;
                mutable tr1::shared_ptr<DependencySpecTree::ConstItem> _c;
                const std::string _v;

            public:
                DepKey(const std::string &, const std::string &, const std::string &, const MetadataKeyType);

                virtual const tr1::shared_ptr<const DependencySpecTree::ConstItem> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
