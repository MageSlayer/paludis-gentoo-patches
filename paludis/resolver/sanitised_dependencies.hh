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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_SANITISED_DEPENDENCIES_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_SANITISED_DEPENDENCIES_HH 1

#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/decider-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/dep_label-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct active_dependency_labels_name> active_dependency_labels;
        typedef Name<struct active_dependency_labels_as_string_name> active_dependency_labels_as_string;
        typedef Name<struct if_package_name> if_package;
        typedef Name<struct if_block_name> if_block;
        typedef Name<struct metadata_key_human_name_name> metadata_key_human_name;
        typedef Name<struct metadata_key_raw_name_name> metadata_key_raw_name;
        typedef Name<struct original_specs_as_string_name> original_specs_as_string;
        typedef Name<struct spec_name> spec;
    }

    namespace resolver
    {
        struct PackageOrBlockDepSpec
        {
            NamedValue<n::if_block, std::tr1::shared_ptr<BlockDepSpec> > if_block;
            NamedValue<n::if_package, std::tr1::shared_ptr<PackageDepSpec> > if_package;

            PackageOrBlockDepSpec(const BlockDepSpec &);
            PackageOrBlockDepSpec(const PackageDepSpec &);

            void serialise(Serialiser &) const;

            static PackageOrBlockDepSpec deserialise(
                    Deserialisation & d,
                    const std::tr1::shared_ptr<const PackageID> & for_id) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        struct SanitisedDependency
        {
            NamedValue<n::active_dependency_labels, std::tr1::shared_ptr<const DependenciesLabelSequence> > active_dependency_labels;
            NamedValue<n::active_dependency_labels_as_string, std::string> active_dependency_labels_as_string;
            NamedValue<n::metadata_key_human_name, std::string> metadata_key_human_name;
            NamedValue<n::metadata_key_raw_name, std::string> metadata_key_raw_name;
            NamedValue<n::original_specs_as_string, std::string> original_specs_as_string;
            NamedValue<n::spec, PackageOrBlockDepSpec> spec;

            void serialise(Serialiser &) const;

            static SanitisedDependency deserialise(
                    Deserialisation & d,
                    const std::tr1::shared_ptr<const PackageID> & for_id) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class SanitisedDependencies :
            private PrivateImplementationPattern<SanitisedDependencies>
        {
            private:
                void _populate_one(
                        const Decider &,
                        const Resolvent &,
                        const std::tr1::shared_ptr<const PackageID> &,
                        const std::tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > (PackageID::* const) () const
                        );

            public:
                SanitisedDependencies();
                ~SanitisedDependencies();

                void populate(
                        const Decider &,
                        const Resolvent &,
                        const std::tr1::shared_ptr<const PackageID> &);

                void add(const SanitisedDependency & d);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const SanitisedDependency> ConstIterator;

                ConstIterator begin() const;
                ConstIterator end() const;
        };
    }
}

#endif
