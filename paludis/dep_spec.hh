/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/util/pimp.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/named_value.hh>

#include <paludis/changed_choices-fwd.hh>
#include <paludis/dep_label.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/dep_spec_data-fwd.hh>
#include <paludis/dep_spec_annotations-fwd.hh>
#include <paludis/package_dep_spec_requirement-fwd.hh>

#include <memory>

/** \file
 * Declarations for dependency spec classes.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_label.cc "example_dep_label.cc" (for labels)
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
        public virtual Cloneable<DepSpec>
    {
        private:
            Pimp<DepSpec> _imp;

        protected:
            DepSpec();

        public:
            ///\name Basic operations
            ///\{

            virtual ~DepSpec();

            DepSpec(const DepSpec &) = delete;
            DepSpec & operator= (const DepSpec &) = delete;

            ///\}

            /**
             * Our annotations, may be null.
             *
             * \since 0.58
             */
            const std::shared_ptr<const DepSpecAnnotations> maybe_annotations() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Change our annotations, may be null.
             *
             * \since 0.58
             */
            void set_annotations(const std::shared_ptr<const DepSpecAnnotations> &);
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

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Represents a ^^ ( first second third ) group of requirements.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     * \since 0.56
     */
    class PALUDIS_VISIBLE ExactlyOneDepSpec :
        public DepSpec
    {
        public:
            ///\name Basic operations
            ///\{

            ExactlyOneDepSpec();

            ///\}

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        public CloneUsingThis<DepSpec, ConditionalDepSpec>
    {
        friend std::ostream & operator<< (std::ostream &, const ConditionalDepSpec &);

        private:
            Pimp<ConditionalDepSpec> _imp;

            std::string _as_string() const;

        public:
            ///\name Basic operations
            ///\{

            ConditionalDepSpec(const std::shared_ptr<const ConditionalDepSpecData> &);
            ConditionalDepSpec(const ConditionalDepSpec &);
            ~ConditionalDepSpec();

            ///\}

            /**
             * Is our condition met?
             *
             * This takes into account inverses etc.
             *
             * \since 0.58 takes env, package_id
             */
            bool condition_met(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Would our condition met, if certain choices were changed?
             *
             * \since 0.58 takes env, package_id
             */
            bool condition_would_be_met_when(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &,
                    const ChangedChoices &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Is our condition meetable?
             *
             * This takes into account inverses, masks, forces etc.
             *
             * \since 0.58 takes env, package_id
             */
            bool condition_meetable(
                    const Environment * const,
                    const std::shared_ptr<const PackageID> &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our data.
             *
             * This shouldn't generally be used by clients, but some repositories use it
             * to gain access to additional data stored in the ConditionalDepSpecData.
             */
            const std::shared_ptr<const ConditionalDepSpecData> data() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        public:
            /**
             * Fetch our text.
             */
            virtual const std::string text() const = 0;
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
        public CloneUsingThis<DepSpec, PackageDepSpec>
    {
        friend std::ostream & operator<< (std::ostream &, const PackageDepSpec &);

        private:
            const PackageDepSpec & operator= (const PackageDepSpec &);

            Pimp<PackageDepSpec> _imp;

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
            PackageDepSpec(const std::shared_ptr<const PackageDepSpecData> &);

            PackageDepSpec(const PackageDepSpec &);

            ~PackageDepSpec();

            ///\}

            virtual const std::string text() const;

            /**
             * Fetch the single NameRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const NameRequirement> package_name_requirement() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the single PackageNamePartRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const PackageNamePartRequirement> package_name_part_requirement() const;

            /**
             * Fetch the single CategoryNamePartRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const CategoryNamePartRequirement> category_name_part_requirement() const;

            /**
             * Fetch all our VersionRequirements, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const VersionRequirementSequence> all_version_requirements() const;

            /**
             * Fetch the single ExactSlotRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const ExactSlotRequirement> exact_slot_requirement() const;

            /**
             * Fetch the single AnySlotRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const AnySlotRequirement> any_slot_requirement() const;

            /**
             * Fetch the single InRepositoryRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InRepositoryRequirement> in_repository_requirement() const;

            /**
             * Fetch the single InstallableToRepositoryRequirement, if we have one, or
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstallableToRepositoryRequirement> installable_to_repository_requirement() const;

            /**
             * Fetch the single FromRepositoryRequirement, if we have one, or
             * a null pointer otherwise.
             */
            const std::shared_ptr<const FromRepositoryRequirement> from_repository_requirement() const;

            /**
             * Fetch the single InstalledAtPathRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstalledAtPathRequirement> installed_at_path_requirement() const;

            /**
             * Fetch the single InstallableToPathRequirement, if we have one, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const InstallableToPathRequirement> installable_to_path_requirement() const;

            /**
             * Fetch all our KeyRequirements, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const KeyRequirementSequence> all_key_requirements() const;

            /**
             * Fetch all our ChoiceRequirements, if we have any, or
             * a null pointer otherwise.
             *
             * \since 0.61
             */
            const std::shared_ptr<const ChoiceRequirementSequence> all_choice_requirements() const;

            /**
             * Fetch all our requirements.
             *
             * \since 0.61
             */
            const std::shared_ptr<const PackageDepSpecRequirementSequence> requirements() const;

            /**
             * Access to our data.
             */
            const std::shared_ptr<const PackageDepSpecData> data() const;
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
        private:
            std::string _text;

        public:
            ///\name Basic operations
            ///\{

            PlainTextDepSpec(const std::string &);

            ///\}

            virtual const std::string text() const;

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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

            virtual const std::string text() const;

            /// Fetch the name of our set.
            const SetName name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private:
            std::string _text;

        public:
            ///\name Basic operations
            ///\{

            LicenseDepSpec(const std::string &);

            ///\}

            virtual const std::string text() const;

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private:
            std::string _text;

        public:
            ///\name Basic operations
            ///\{

            FetchableURIDepSpec(const std::string &);

            ///\}

            virtual const std::string text() const;

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

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private:
            std::string _text;

        public:
            ///\name Basic operations
            ///\{

            SimpleURIDepSpec(const std::string &);

            ///\}

            virtual const std::string text() const;

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
            std::string _text;
            PackageDepSpec _spec;

        public:
            ///\name Basic operations
            ///\{

            BlockDepSpec(const std::string & text, const PackageDepSpec & spec);

            BlockDepSpec(const BlockDepSpec &);

            ///\}

            virtual const std::string text() const;

            /**
             * Fetch the spec we're blocking.
             *
             * \since 0.41
             */
            const PackageDepSpec blocking() const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
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
        public DepSpec
    {
        private:
            Pimp<LabelsDepSpec> _imp;

        public:
            ///\name Basic operations
            ///\{

            LabelsDepSpec();
            ~LabelsDepSpec();

            ///\}

            ///\name Contained labels
            ///\{

            void add_label(const std::shared_ptr<const Labels_> &);

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag,
                    const std::shared_ptr<const Labels_> > ConstIterator;

            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE PlainTextLabelDepSpec :
        public StringDepSpec
    {
        private:
            std::string _text;

        public:
            ///\name Basic operations
            ///\{

            PlainTextLabelDepSpec(const std::string &);
            ~PlainTextLabelDepSpec();

            ///\}

            virtual const std::string text() const;

            virtual std::shared_ptr<DepSpec> clone() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string label() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class Cloneable<DepSpec>;
    extern template class Pimp<ConditionalDepSpec>;
    extern template class CloneUsingThis<DepSpec, ConditionalDepSpec>;
    extern template class Pimp<PackageDepSpec>;
    extern template class CloneUsingThis<DepSpec, PackageDepSpec>;
    extern template class Pimp<DependenciesLabelsDepSpec>;
    extern template class Pimp<URILabelsDepSpec>;
    extern template class Pimp<PlainTextLabelDepSpec>;

    extern template class WrappedForwardIterator<DependenciesLabelsDepSpec::ConstIteratorTag,
           const std::shared_ptr<const DependenciesLabel> >;
    extern template class WrappedForwardIterator<URILabelsDepSpec::ConstIteratorTag,
           const std::shared_ptr<const URILabel> >;

}

#endif
