/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/package_or_block_dep_spec.hh>
#include <paludis/resolver/labels_classifier.hh>
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
        typedef Name<struct name_active_conditions_as_string> active_conditions_as_string;
        typedef Name<struct name_active_dependency_labels> active_dependency_labels;
        typedef Name<struct name_active_dependency_labels_as_string> active_dependency_labels_as_string;
        typedef Name<struct name_active_dependency_labels_classifier> active_dependency_labels_classifier;
        typedef Name<struct name_from_id> from_id;
        typedef Name<struct name_metadata_key_human_name> metadata_key_human_name;
        typedef Name<struct name_metadata_key_raw_name> metadata_key_raw_name;
        typedef Name<struct name_original_specs_as_string> original_specs_as_string;
        typedef Name<struct name_spec> spec;
    }

    namespace resolver
    {
        struct SanitisedDependency
        {
            NamedValue<n::active_conditions_as_string, std::string> active_conditions_as_string;
            NamedValue<n::active_dependency_labels, std::shared_ptr<const DependenciesLabelSequence> > active_dependency_labels;
            NamedValue<n::active_dependency_labels_as_string, std::string> active_dependency_labels_as_string;
            NamedValue<n::active_dependency_labels_classifier, std::shared_ptr<const LabelsClassifier> > active_dependency_labels_classifier;
            NamedValue<n::from_id, std::shared_ptr<const PackageID> > from_id;
            NamedValue<n::metadata_key_human_name, std::string> metadata_key_human_name;
            NamedValue<n::metadata_key_raw_name, std::string> metadata_key_raw_name;
            NamedValue<n::original_specs_as_string, std::string> original_specs_as_string;
            NamedValue<n::spec, PackageOrBlockDepSpec> spec;

            void serialise(Serialiser &) const;

            static SanitisedDependency deserialise(
                    Deserialisation & d,
                    const std::shared_ptr<const PackageID> & for_id) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class SanitisedDependencies
        {
            private:
                Pimp<SanitisedDependencies> _imp;

            private:
                void _populate_one(
                        const Environment * const,
                        const Decider &,
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const ChangedChoices> &,
                        const std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > (PackageID::* const) () const
                        );

            public:
                SanitisedDependencies();
                ~SanitisedDependencies();

                void populate(
                        const Environment * const,
                        const Decider &,
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const ChangedChoices> &);

                void add(const SanitisedDependency & d);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const SanitisedDependency> ConstIterator;

                ConstIterator begin() const;
                ConstIterator end() const;
        };
    }
}

#endif
