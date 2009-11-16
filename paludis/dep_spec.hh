/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>

#include <paludis/dep_label.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tag-fwd.hh>
#include <paludis/name.hh>
#include <paludis/metadata_key_holder.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_requirements-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/slot_requirement-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

#include <tr1/memory>
#include <tr1/functional>

/** \file
 * Declarations for dependency spec classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_label.cc "example_dep_label.cc" (for labels)
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
        private PrivateImplementationPattern<DepSpec>,
        public MetadataKeyHolder,
        public virtual Cloneable<DepSpec>
    {
        private:
            PrivateImplementationPattern<DepSpec>::ImpPtr & _imp;

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

            /**
             * The annotations_key, if non-zero, contains any annotations.
             */
            const std::tr1::shared_ptr<const MetadataSectionKey> annotations_key() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Change the annotations key.
             */
            void set_annotations_key(const std::tr1::shared_ptr<const MetadataSectionKey> &);

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
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            AnyDepSpec();

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            AllDepSpec();

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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

            ConditionalDepSpec(const std::tr1::shared_ptr<const ConditionalDepSpecData> &);
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
            const std::tr1::shared_ptr<const ConditionalDepSpecData> data() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
     * An additional requirement for a PackageDepSpec.
     *
     * \since 0.26
     * \ingroup g_dep_spec
     */
    class PALUDIS_VISIBLE AdditionalPackageDepSpecRequirement :
        private InstantiationPolicy<AdditionalPackageDepSpecRequirement, instantiation_method::NonCopyableTag>
    {
        public:
            virtual ~AdditionalPackageDepSpecRequirement();

            /**
             * Is our requirement met for a given PackageID?
             */
            virtual bool requirement_met(const Environment * const, const PackageID &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a human readable string representation of ourself.
             */
            virtual const std::string as_human_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a raw string representation of ourself.
             */
            virtual const std::string as_raw_string() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    namespace n
    {
        struct include_masked;
        struct path;
        struct repository;
    }

    /**
     * Data for PackageDepSpec.installable_to_repository_ptr() etc.
     *
     * \ingroup g_dep_spec
     * \since 0.32
     */
    struct InstallableToRepository
    {
        NamedValue<n::include_masked, bool> include_masked;
        NamedValue<n::repository, RepositoryName> repository;
    };

    /**
     * Data for PackageDepSpec.installable_to_path_ptr() etc.
     *
     * \ingroup g_dep_spec
     * \since 0.32
     */
    struct InstallableToPath
    {
        NamedValue<n::include_masked, bool> include_masked;
        NamedValue<n::path, FSEntry> path;
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

            PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpecOptions &);
            ~PartiallyMadePackageDepSpec();
            PartiallyMadePackageDepSpec(const PackageDepSpec &);
            PartiallyMadePackageDepSpec(const PartiallyMadePackageDepSpec &);

            ///\}

            /**
             * Set our package requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & package(const QualifiedPackageName &);

            /**
             * Set our slot requirements, return ourself.
             */
            PartiallyMadePackageDepSpec & slot_requirement(const std::tr1::shared_ptr<const SlotRequirement> &);

            /**
             * Set our in-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & in_repository(const RepositoryName &);

            /**
             * Set our from-repository requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & from_repository(const RepositoryName &);

            /**
             * Set our installable-to-repository requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installable_to_repository(const InstallableToRepository &);

            /**
             * Set our installed-at-path requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installed_at_path(const FSEntry &);

            /**
             * Set our installable-to-path requirement, return ourself.
             *
             * \since 0.32
             */
            PartiallyMadePackageDepSpec & installable_to_path(const InstallableToPath &);

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
             * Add an additional requirement, return ourself.
             */
            PartiallyMadePackageDepSpec & additional_requirement(
                    const std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirement> &);

            /**
             * Clear additional requirements, return ourself.
             *
             * \since 0.41
             */
            PartiallyMadePackageDepSpec & clear_additional_requirements();

            /**
             * Add annotations
             */
            PartiallyMadePackageDepSpec & annotations(
                    const std::tr1::shared_ptr<const MetadataSectionKey> &);

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

            PrivateImplementationPattern<PackageDepSpec>::ImpPtr & _imp;

        protected:
            virtual void need_keys_added() const;

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
            PackageDepSpec(const std::tr1::shared_ptr<const PackageDepSpecData> &);

            PackageDepSpec(const PackageDepSpec &);

            ~PackageDepSpec();

            ///\}

            /**
             * Fetch the package name (may be a zero pointer).
             */
            std::tr1::shared_ptr<const QualifiedPackageName> package_ptr() const;

            /**
             * Fetch the package name part, if wildcarded, or a zero pointer otherwise.
             */
            std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const;

            /**
             * Fetch the category name part, if wildcarded, or a zero pointer otherwise.
             */
            std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            std::tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const;

            /**
             * Fetch the version requirements mode.
             */
            VersionRequirementsMode version_requirements_mode() const;

            /**
             * Fetch the slot requirement (may be a zero pointer).
             */
            std::tr1::shared_ptr<const SlotRequirement> slot_requirement_ptr() const;

            /**
             * Fetch the in-repository requirement (may be a zero pointer).
             */
            std::tr1::shared_ptr<const RepositoryName> in_repository_ptr() const;

            /**
             * Fetch the installable-to-repository requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            std::tr1::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const;

            /**
             * Fetch the from-repository requirement (may be a zero pointer).
             */
            std::tr1::shared_ptr<const RepositoryName> from_repository_ptr() const;

            /**
             * Fetch the installed-at-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            std::tr1::shared_ptr<const FSEntry> installed_at_path_ptr() const;

            /**
             * Fetch the installable-to-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            std::tr1::shared_ptr<const InstallableToPath> installable_to_path_ptr() const;

            /**
             * Fetch any additional requirements (may be a zero pointer).
             */
            std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const;

            /**
             * Fetch our tag.
             */
            std::tr1::shared_ptr<const DepTag> tag() const;

            /**
             * Set our tag.
             */
            void set_tag(const std::tr1::shared_ptr<const DepTag> & s);

            /**
             * Fetch a copy of ourself without additional requirements.
             */
            std::tr1::shared_ptr<PackageDepSpec> without_additional_requirements() const;

            /**
             * Access to our data.
             */
            std::tr1::shared_ptr<const PackageDepSpecData> data() const;

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
            virtual std::tr1::shared_ptr<const QualifiedPackageName> package_ptr() const = 0;

            /**
             * Fetch the package name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const = 0;

            /**
             * Fetch the category name part, if wildcarded, or a zero pointer otherwise.
             */
            virtual std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const = 0;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const = 0;

            /**
             * Fetch the version requirements mode.
             */
            virtual VersionRequirementsMode version_requirements_mode() const = 0;

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const SlotRequirement> slot_requirement_ptr() const = 0;

            /**
             * Fetch the from-repository requirement (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const RepositoryName> in_repository_ptr() const = 0;

            /**
             * Fetch the installable-to-repository requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::tr1::shared_ptr<const InstallableToRepository> installable_to_repository_ptr() const = 0;

            /**
             * Fetch the from-repository requirement (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const RepositoryName> from_repository_ptr() const = 0;

            /**
             * Fetch the installed-at-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::tr1::shared_ptr<const FSEntry> installed_at_path_ptr() const = 0;

            /**
             * Fetch the installable-to-path requirement (may be a zero pointer).
             *
             * \since 0.32
             */
            virtual std::tr1::shared_ptr<const InstallableToPath> installable_to_path_ptr() const = 0;

            /**
             * Fetch the additional requirements (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const = 0;

            /**
             * Fetch the annotations (may be a zero pointer).
             */
            virtual std::tr1::shared_ptr<const MetadataSectionKey> annotations_key() const = 0;

            /**
             * Fetch options if we're being used to construct a new PartiallyMadePackageDepSpec.
             *
             * \since 0.38
             */
            virtual const PartiallyMadePackageDepSpecOptions options_for_partially_made_package_dep_spec() const = 0;
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
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            PlainTextDepSpec(const std::string &);

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            NamedSetDepSpec(const SetName &);

            ///\}

            /// Fetch the name of our set.
            const SetName name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            LicenseDepSpec(const std::string &);

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        protected:
            virtual void need_keys_added() const;

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

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            SimpleURIDepSpec(const std::string &);

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
            PackageDepSpec _spec;
            bool _strong;

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            /**
             * \since 0.41
             */
            BlockDepSpec(const std::string & text, const PackageDepSpec & spec, const bool strong);

            BlockDepSpec(const BlockDepSpec &);

            ///\}

            /**
             * Fetch the spec we're blocking.
             *
             * \since 0.41
             */
            const PackageDepSpec blocking() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch whether we're a strong blocker.
             *
             * \since 0.41
             */
            bool strong() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
    template <typename Labels_>
    class PALUDIS_VISIBLE LabelsDepSpec :
        public DepSpec,
        private PrivateImplementationPattern<LabelsDepSpec<Labels_> >
    {
        private:
            typename PrivateImplementationPattern<LabelsDepSpec>::ImpPtr & _imp;

        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            LabelsDepSpec();
            ~LabelsDepSpec();

            ///\}

            ///\name Contained labels
            ///\{

            void add_label(const std::tr1::shared_ptr<const Labels_> &);

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag,
                    const std::tr1::shared_ptr<const Labels_> > ConstIterator;

            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PlainTextLabelDepSpec :
        public StringDepSpec
    {
        protected:
            virtual void need_keys_added() const;

        public:
            ///\name Basic operations
            ///\{

            PlainTextLabelDepSpec(const std::string &);
            ~PlainTextLabelDepSpec();

            ///\}

            virtual std::tr1::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string label() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class Cloneable<DepSpec>;
    extern template class PrivateImplementationPattern<ConditionalDepSpec>;
    extern template class CloneUsingThis<DepSpec, ConditionalDepSpec>;
    extern template class PrivateImplementationPattern<PartiallyMadePackageDepSpec>;
    extern template class PrivateImplementationPattern<PackageDepSpec>;
    extern template class CloneUsingThis<DepSpec, PackageDepSpec>;
    extern template class PrivateImplementationPattern<DependenciesLabelsDepSpec>;
    extern template class PrivateImplementationPattern<URILabelsDepSpec>;
    extern template class PrivateImplementationPattern<PlainTextLabelDepSpec>;
#endif
}

#endif
