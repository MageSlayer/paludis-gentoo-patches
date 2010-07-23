/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

        class PackageIDSequenceKey :
            public MetadataCollectionKey<PackageIDSequence>
        {
            private:
                const Environment * const _env;
                const std::shared_ptr<PackageIDSequence> _v;

                const std::string _r;
                const std::string _h;
                const MetadataKeyType _t;

            public:
                PackageIDSequenceKey(const Environment * const,
                        const std::string &, const std::string &, const MetadataKeyType);

                virtual const std::shared_ptr<const PackageIDSequence> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                void push_back(const std::shared_ptr<const PackageID> &);

                virtual std::string pretty_print_flat(const Formatter<PackageID> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PackageIDKey :
            public MetadataValueKey<std::shared_ptr<const PackageID> >
        {
            private:
                const CRANPackageID * const _v;

                const std::string _r;
                const std::string _h;
                const MetadataKeyType _t;

            public:
                PackageIDKey(const std::string &, const std::string &, const CRANPackageID * const, const MetadataKeyType);

                virtual const std::shared_ptr<const PackageID> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const Formatter<PackageID> &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class DepKey :
            public MetadataSpecTreeKey<DependencySpecTree>,
            private Pimp<DepKey>
        {
            private:
                Pimp<DepKey>::ImpPtr & _imp;

            public:
                DepKey(const Environment * const,
                        const std::string &, const std::string &, const std::string &,
                        const std::shared_ptr<const DependenciesLabelSequence> &, const MetadataKeyType);

                ~DepKey();

                virtual const std::shared_ptr<const DependencySpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual std::string pretty_print_flat(const DependencySpecTree::ItemFormatter &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
