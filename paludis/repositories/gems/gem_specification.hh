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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATION_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEM_SPECIFICATION_HH 1

#include <paludis/repositories/gems/gem_specification-fwd.hh>
#include <paludis/repositories/gems/yaml-fwd.hh>
#include <paludis/package_id.hh>
#include <paludis/name-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <string>

namespace paludis
{
    namespace gems
    {
        /**
         * Thrown if a bad Gem specification is encountered.
         *
         * \ingroup grpexceptions
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE BadSpecificationError :
            public Exception
        {
            public:
                ///\name Basic operations
                ///\{

                BadSpecificationError(const std::string &) throw ();

                ///\}
        };

        /**
         * Represents a Gem specification.
         *
         * \ingroup grpgemsrepository
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE GemSpecification :
            private PrivateImplementationPattern<GemSpecification>,
            public PackageID
        {
            private:
                Implementation<GemSpecification> * const _imp;

            protected:
                void need_keys_added() const;

            public:
                ///\name Basic operations
                ///\{

                GemSpecification(const tr1::shared_ptr<const Repository> &, const yaml::Node &);
                GemSpecification(const tr1::shared_ptr<const Repository> &, const PackageNamePart &,
                        const VersionSpec &, const FSEntry &);

                ~GemSpecification();

                ///\}

                /* PackageID */
                virtual const std::string canonical_form(const PackageIDCanonicalForm) const;

                virtual const QualifiedPackageName name() const;
                virtual const VersionSpec version() const;
                virtual const SlotName slot() const;
                virtual const tr1::shared_ptr<const Repository> repository() const;
                virtual const tr1::shared_ptr<const EAPI> eapi() const;

                virtual const tr1::shared_ptr<const MetadataPackageIDKey> virtual_for_key() const;
                virtual const tr1::shared_ptr<const MetadataSetKey<KeywordNameSet> > keywords_key() const;
                virtual const tr1::shared_ptr<const MetadataSetKey<IUseFlagSet> > iuse_key() const;
                virtual const tr1::shared_ptr<const MetadataSetKey<UseFlagNameSet> > use_key() const;
                virtual const tr1::shared_ptr<const MetadataSetKey<InheritedSet> > inherited_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<LicenseSpecTree> > license_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<ProvideSpecTree> > provide_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > build_dependencies_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > run_dependencies_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > post_dependencies_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > suggested_dependencies_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<RestrictSpecTree> > restrict_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > src_uri_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > homepage_key() const;
                virtual const tr1::shared_ptr<const MetadataSpecTreeKey<URISpecTree> > bin_uri_key() const;
                virtual const tr1::shared_ptr<const MetadataStringKey> short_description_key() const;
                virtual const tr1::shared_ptr<const MetadataStringKey> long_description_key() const;

                virtual const tr1::shared_ptr<const MetadataContentsKey> contents_key() const;
                virtual const tr1::shared_ptr<const MetadataTimeKey> installed_time_key() const;
                virtual const tr1::shared_ptr<const MetadataStringKey> source_origin_key() const;
                virtual const tr1::shared_ptr<const MetadataStringKey> binary_origin_key() const;

                virtual bool arbitrary_less_than_comparison(const PackageID &) const;
                virtual std::size_t extra_hash_value() const;
        };
    }
}

#endif
