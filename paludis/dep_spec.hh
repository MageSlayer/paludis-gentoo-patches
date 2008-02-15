/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_DEP_SPEC_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/clone.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <paludis/dep_label.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tag-fwd.hh>
#include <paludis/name.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_requirements-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/use_requirements-fwd.hh>

/** \file
 * Declarations for dependency spec classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_label.cc "example_dep_label.cc" (for labels)
 * - \ref example_dep_tree.cc "example_dep_tree.cc" (for specification trees)
 * - \ref example_dep_tag.cc "example_dep_tag.cc" (for tags)
 */

namespace paludis
{
    /**
     * Base class for a dependency spec.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepSpec :
        private InstantiationPolicy<DepSpec, instantiation_method::NonCopyableTag>,
        public virtual Cloneable<DepSpec>
    {
        protected:
            DepSpec();

        public:
            ///\name Basic operations
            ///\{

            virtual ~DepSpec();

            ///\}

            ///\name Upcasts
            ///\{

            /**
             * Return us as a ConditionalDepSpec, or 0 if we are not a
             * ConditionalDepSpec.
             */
            virtual const ConditionalDepSpec * as_conditional_dep_spec() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return us as a PackageDepSpec, or 0 if we are not a
             * ConditionalDepSpec.
             */
            virtual const PackageDepSpec * as_package_dep_spec() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Represents a "|| ( )" dependency block.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AnyDepSpec :
        public DepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            AnyDepSpec();

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a ( first second third ) or top level group of dependency
     * specs.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AllDepSpec :
        public DepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            AllDepSpec();

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a dependency spec whose children should only be considered
     * upon a certain condition (for example, a use dependency block).
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ConditionalDepSpec :
        public DepSpec,
        private PrivateImplementationPattern<ConditionalDepSpec>,
        public MetadataKeyHolder,
        public CloneUsingThis<DepSpec, ConditionalDepSpec>
    {
        friend std::ostream & operator<< (std::ostream &, const ConditionalDepSpec &);

        private:
            PrivateImplementationPattern<ConditionalDepSpec>::ImpPtr & _imp;

            std::string _as_string() const;

        protected:
            virtual void need_keys_added() const;
            virtual void clear_metadata_keys() const;

        public:
            ///\name Basic operations
            ///\{

            ConditionalDepSpec(const tr1::shared_ptr<const ConditionalDepSpecData> &);
            ConditionalDepSpec(const ConditionalDepSpec &);
            ~ConditionalDepSpec();

            ///\}

            virtual const ConditionalDepSpec * as_conditional_dep_spec() const;

            /**
             * Is our condition met?
             *
             * This takes into account inverses etc.
             */
            bool condition_met() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is our condition meetable?
             *
             * This takes into account inverses, masks, forces etc.
             */
            bool condition_meetable() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our data.
             *
             * This shouldn't generally be used by clients, but some repositories use it
             * to gain access to additional data stored in the ConditionalDepSpecData.
             */
            const tr1::shared_ptr<const ConditionalDepSpecData> data() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Data for a ConditionalDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE ConditionalDepSpecData :
        public MetadataKeyHolder
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~ConditionalDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            virtual std::string as_string() const = 0;

            /**
             * Fetch the result for condition_met.
             */
            virtual bool condition_met() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Fetch the result for condition_meetable.
             */
            virtual bool condition_meetable() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A StringDepSpec represents a dep spec with an associated piece of text.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE StringDepSpec :
        public DepSpec
    {
        private:
            std::string _str;

        protected:
            ///\name Basic operations
            ///\{

            StringDepSpec(const std::string &);

            ~StringDepSpec();

            ///\}

            /**
             * Change our text.
             */
            void set_text(const std::string &);

        public:
            /**
             * Fetch our text.
             */
            std::string text() const;
    };

    /**
     * A PartiallyMadePackageDepSpec is returned by make_package_dep_spec()
     * and is used to incrementally build a PackageDepSpec.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     */
    class PALUDIS_VISIBLE PartiallyMadePackageDepSpec :
        private PrivateImplementationPattern<PartiallyMadePackageDepSpec>
    {
        public:
            ///\name Basic operations
            ///\{

            PartiallyMadePackageDepSpec();
            ~PartiallyMadePackageDepSpec();
            PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec &);

            ///\}

            /**
             * Set our package requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package(const QualifiedPackageName &);

            /**
             * Set our slot requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & slot(const SlotName &);

            /**
             * Set our repository requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & repository(const RepositoryName &);

            /**
             * Set our package name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package_name_part(const PackageNamePart &);

            /**
             * Set our category name part requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & category_name_part(const CategoryNamePart &);

            /**
             * Add a version requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & version_requirement(const VersionRequirement &);

            /**
             * Set our version requirements mode, return ourself.
             */
            PartiallyMadePackageDepSpec & version_requirements_mode(const VersionRequirementsMode &);

            /**
             * Add a use requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & use_requirement(const tr1::shared_ptr<const UseRequirement> &);

            /**
             * Turn ourselves into a PackageDepSpec.
             */
            operator const PackageDepSpec() const;

            /**
             * Explicitly turn ourselves into a PackageDepSpec.
             */
            const PackageDepSpec to_package_dep_spec() const;
    };

    /**
     * A PackageDepSpec represents a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * A PackageDepSpec is implemented in terms of PackageDepSpecData. Individual
     * repositories provide their own way of creating PackageDepSpec::Data that
     * handle the native syntax for those repositories (e.g. CRAN uses
     * "Blah (>= 1.23)" whilst E uses ">=cat/blah-1.23").
     *
     * To create a PackageDepSpec from user input, use
     * parse_user_package_dep_spec(), and for programmer input, use
     * make_package_dep_spec().
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PackageDepSpec :
        public StringDepSpec,
        private PrivateImplementationPattern<PackageDepSpec>,
        public CloneUsingThis<DepSpec, PackageDepSpec>
    {
        friend std::ostream & operator<< (std::ostream &, const PackageDepSpec &);

        private:
            const PackageDepSpec & operator= (const PackageDepSpec &);
            std::string _as_string() const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor.
             *
             * Clients will usually use either parse_user_package_dep_spec() or
             * make_package_dep_spec() rather than calling this method
             * directly. Repositories will define their own way of creating
             * a PackageDepSpec.
             *
             * \since 0.26
             */
            PackageDepSpec(const tr1::shared_ptr<const PackageDepSpecData> &);

            PackageDepSpec(const PackageDepSpec &);

            ~PackageDepSpec();

            ///\}

            /**
             * Fetch the package name (may be a zero pointer).
             */
            tr1::shared_ptr<const QualifiedPackageName> package_ptr() const;

            /**
             * Fetch the package name part, if wildcarded, or a zero pointer otherwise.
             */
            tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const;

            /**
             * Fetch the category name part, if wildcarded, or a zero pointer otherwise.
             */
            tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const;

            /**
             * Fetch the version requirements mode.
             */
            VersionRequirementsMode version_requirements_mode() const;

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            tr1::shared_ptr<const SlotName> slot_ptr() const;

            /**
             * Fetch the repo name (may be a zero pointer).
             */
            tr1::shared_ptr<const RepositoryName> repository_ptr() const;

            /**
             * Fetch the use requirements (may be a zero pointer).
             */
            tr1::shared_ptr<const UseRequirements> use_requirements_ptr() const;

            /**
             * Fetch our tag.
             */
            tr1::shared_ptr<const DepTag> tag() const;

            /**
             * Set our tag.
             */
            void set_tag(const tr1::shared_ptr<const DepTag> & s);

            /**
             * Fetch a copy of ourself without the USE requirements.
             */
            tr1::shared_ptr<PackageDepSpec> without_use_requirements() const;

            virtual const PackageDepSpec * as_package_dep_spec() const;
    };

    /**
     * Data for a PackageDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE PackageDepSpecData
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~PackageDepSpecData();

            ///\}

            /**
             * Fetch ourself as a string.
             */
            virtual std::string as_string() const = 0;

            /**
             * Fetch the package name (may be a zero pointer).
             */
            virtual tr1::shared_ptr<const QualifiedPackageName> package_ptr() const = 0;

            /**
             * Fetch the package name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const = 0;

            /**
             * Fetch the category name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const = 0;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            virtual tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const = 0;

            /**
             * Fetch the version requirements mode.
             */
            virtual VersionRequirementsMode version_requirements_mode() const = 0;

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            virtual tr1::shared_ptr<const SlotName> slot_ptr() const = 0;

            /**
             * Fetch the repo name (may be a zero pointer).
             */
            virtual tr1::shared_ptr<const RepositoryName> repository_ptr() const = 0;

            /**
             * Fetch the use requirements (may be a zero pointer).
             */
            virtual tr1::shared_ptr<const UseRequirements> use_requirements_ptr() const = 0;
    };

    /**
     * A PlainTextDepSpec represents a plain text entry.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PlainTextDepSpec :
        public StringDepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            PlainTextDepSpec(const std::string &);

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A NamedSetDepSpec represents a named package set.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NamedSetDepSpec :
        public StringDepSpec
    {
        private:
            const SetName _name;

        public:
            ///\name Basic operations
            ///\{

            NamedSetDepSpec(const SetName &);

            ///\}

            /// Fetch the name of our set.
            const SetName name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LicenseDepSpec represents a license entry.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE LicenseDepSpec :
        public StringDepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            LicenseDepSpec(const std::string &);

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A FetchableURIDepSpec represents a fetchable URI part.
     *
     * It differs from a SimpleURIDepSpec in that it supports arrow notation. Arrows
     * are used by exheres to allow downloading to a filename other than that used by
     * the original URL.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FetchableURIDepSpec :
        public StringDepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            FetchableURIDepSpec(const std::string &);

            ///\}

            /**
             * The original URL (that is, the text to the left of the arrow, if present,
             * or the entire text otherwise).
             */
            std::string original_url() const;

            /**
             * The renamed URL filename (that is, the text to the right of the arrow,
             * if present, or an empty string otherwise).
             */
            std::string renamed_url_suffix() const;

            /**
             * The filename (that is, the renamed URL suffix, if present, or the text
             * after the final / in the original URL otherwise).
             */
            std::string filename() const;

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A SimpleURIDepSpec represents a simple URI part.
     *
     * Unlike FetchableURIDepSpec, arrow notation is not supported.
     *
     * \ingroup g_dep_spec
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SimpleURIDepSpec :
        public StringDepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            SimpleURIDepSpec(const std::string &);

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Thrown if an invalid package dep spec specification is encountered.
     *
     * \ingroup g_exceptions
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PackageDepSpecError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            PackageDepSpecError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * A BlockDepSpec represents a block on a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE BlockDepSpec :
        public StringDepSpec
    {
        private:
            tr1::shared_ptr<const PackageDepSpec> _spec;

        public:
            ///\name Basic operations
            ///\{

            BlockDepSpec(tr1::shared_ptr<const PackageDepSpec> spec);

            ///\}

            /**
             * Fetch the spec we're blocking.
             */
            tr1::shared_ptr<const PackageDepSpec> blocked_spec() const;

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A LabelsDepSpec represents a labels entry using a particular visitor
     * types class.
     *
     * \see DependencyLabelsDepSpec
     * \see URILabelsDepSpec
     * \since 0.26
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    template <typename SpecTree_>
    class PALUDIS_VISIBLE LabelsDepSpec :
        public DepSpec,
        private PrivateImplementationPattern<LabelsDepSpec<SpecTree_> >
    {
        private:
            using PrivateImplementationPattern<LabelsDepSpec<SpecTree_> >::_imp;

        public:
            ///\name Basic operations
            ///\{

            LabelsDepSpec();
            ~LabelsDepSpec();

            ///\}

            ///\name Contained labels
            ///\{

            void add_label(const tr1::shared_ptr<const typename SpecTree_::BasicNode> &);

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag,
                    const tr1::shared_ptr<const typename SpecTree_::BasicNode> > ConstIterator;

            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            virtual tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

}

#endif
