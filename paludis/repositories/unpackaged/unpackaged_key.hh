/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp.hh>

namespace paludis
{
    namespace unpackaged_repositories
    {
        class UnpackagedID;

        class UnpackagedDependencyKey :
            public MetadataSpecTreeKey<DependencySpecTree>
        {
            private:
                Pimp<UnpackagedDependencyKey> _imp;

            public:
                UnpackagedDependencyKey(const Environment * const env,
                        const std::string & r, const std::string & h, const MetadataKeyType t,
                        const std::shared_ptr<const DependenciesLabelSequence> &,
                        const std::string & v);
                ~UnpackagedDependencyKey() override;

                const std::shared_ptr<const DependencySpecTree> parse_value() const override;

                const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                    override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class UnpackagedChoicesKey :
            public MetadataValueKey<std::shared_ptr<const Choices> >
        {
            private:
                Pimp<UnpackagedChoicesKey> _imp;

            public:
                UnpackagedChoicesKey(const Environment * const env,
                        const std::string & r, const std::string & h, const MetadataKeyType t,
                        const UnpackagedID * const id);
                ~UnpackagedChoicesKey() override;

                const std::shared_ptr<const Choices> parse_value() const override;

                const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
