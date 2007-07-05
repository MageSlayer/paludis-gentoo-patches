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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_ID_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_ID_HH 1

#include <paludis/package_id-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/operators.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/eapi-fwd.hh>
#include <paludis/repository-fwd.hh>

namespace paludis
{
    class PackageDatabase;

    class PALUDIS_VISIBLE PackageID :
        private InstantiationPolicy<PackageID, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<PackageID>,
        public equality_operators::HasEqualityOperators
    {
        protected:
            virtual void add_key(const tr1::shared_ptr<const MetadataKey> &) const;
            virtual void need_keys_added() const = 0;

        public:
            PackageID();
            virtual ~PackageID() = 0;

            virtual const std::string canonical_form(const PackageIDCanonicalForm) const = 0;

            virtual const QualifiedPackageName name() const = 0;
            virtual const VersionSpec version() const = 0;
            virtual const SlotName slot() const = 0;
            virtual const tr1::shared_ptr<const Repository> repository() const = 0;
            virtual const tr1::shared_ptr<const EAPI> eapi() const = 0;

            ///\name Specific keys
            ///\{

            virtual const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> > keywords_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> > use_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > iuse_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSetKey<InheritedSet> > inherited_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > restrict_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > src_uri_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > bin_uri_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > homepage_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataStringKey> short_description_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataStringKey> long_description_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataContentsKey> contents_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataStringKey> source_origin_key() const = 0;
            virtual const tr1::shared_ptr<const MetadataStringKey> binary_origin_key() const = 0;

            ///\}

            ///\name Finding and iterating over keys
            ///\{

            typedef libwrapiter::ForwardIterator<PackageID, const MetadataKey> Iterator;
            Iterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));
            Iterator find(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Extra comparisons
            ///\{

            /**
             * Perform an arbitrary less than comparison on another PackageID.
             *
             * Used by PackageIDSetComparator and operator==. This function
             * should not be used by anything else.
             *
             * This function will only be called if the other PackageID has the
             * same name, version and repository as this. If this is not enough
             * to uniquely identify an ID (e.g. if there is an affix, or if multiple
             * slots per version are allowed), then this function's
             * implementation must differentiate appropriately.
             */
            virtual bool arbitrary_less_than_comparison(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Provide any additional hash information for a PackageID.
             *
             * The standard PackageID hash incorporates the repository name, the
             * package name and the version of the package. If this function is
             * defined, its value is also used when determining a hash. This can
             * provide increased performance if a repository uses affixes or
             * multiple slots per version.
             */
            virtual std::size_t extra_hash_value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}
    };

    bool operator== (const PackageID &, const PackageID &) PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    class PALUDIS_VISIBLE PackageIDSetComparator
    {
        public:
            bool operator() (const tr1::shared_ptr<const PackageID> &,
                    const tr1::shared_ptr<const PackageID> &) const;
    };

    class PALUDIS_VISIBLE PackageIDComparator :
        private PrivateImplementationPattern<PackageIDComparator>
    {
        public:
            typedef bool result_type;

            PackageIDComparator(const PackageDatabase * const);
            ~PackageIDComparator();

            bool operator() (const tr1::shared_ptr<const PackageID> &,
                    const tr1::shared_ptr<const PackageID> &) const;
    };
}

#endif
