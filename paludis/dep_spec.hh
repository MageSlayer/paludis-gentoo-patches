/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_DEP_ATOM_HH 1

#include <paludis/dep_tag.hh>
#include <paludis/name.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/visitor.hh>
#include <paludis/version_requirements.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <tr1/memory>

/** \file
 * Declarations for the DepSpec classes.
 *
 * \ingroup grpdepspecs
 */

namespace paludis
{
    class DepSpec;
    class CompositeDepSpec;
    class PackageDepSpec;
    class PlainTextDepSpec;
    class AllDepSpec;
    class AnyDepSpec;
    class UseDepSpec;
    class SetDepSpec;
    class BlockDepSpec;

#include <paludis/dep_spec-se.hh>

    /**
     * Visitor types for a visitor that can visit a DepSpec heirarchy.
     *
     * \ingroup grpdepspecs
     */
    typedef VisitorTypes<PackageDepSpec *, PlainTextDepSpec *, AllDepSpec *, AnyDepSpec *,
            UseDepSpec *, BlockDepSpec *> DepSpecVisitorTypes;

    /**
     * Base class for a dependency spec.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class DepSpec :
        public virtual VisitableInterface<DepSpecVisitorTypes>,
        private InstantiationPolicy<DepSpec, instantiation_method::NonCopyableTag>
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
             * Return us as a UseDepSpec, or 0 if we are not a
             * UseDepSpec.
             */
            virtual const UseDepSpec * as_use_dep_spec() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return us as a PackageDepSpec, or 0 if we are not a
             * UseDepSpec.
             */
            virtual const PackageDepSpec * as_package_dep_spec() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Class for dependency specs that have a number of child dependency
     * specs.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class CompositeDepSpec :
        public DepSpec,
        private PrivateImplementationPattern<CompositeDepSpec>
    {
        protected:
            ///\name Basic operations
            ///\{

            CompositeDepSpec();

            ///\}

        public:
            ///\name Basic operations
            ///\{

            ~CompositeDepSpec();

            ///\}

            ///\name Modify our children
            ///\{

            /**
             * Append a child to our collection.
             */
            virtual void add_child(std::tr1::shared_ptr<const DepSpec>);

            ///\}

            ///\name Iterate over our children
            ///\{

            typedef libwrapiter::ForwardIterator<CompositeDepSpec, const std::tr1::shared_ptr<const DepSpec> > Iterator;

            Iterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            Iterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Represents a "|| ( )" dependency block.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class AnyDepSpec :
        public CompositeDepSpec,
        public Visitable<AnyDepSpec, DepSpecVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            AnyDepSpec();

            ///\}
    };

    /**
     * Represents a ( first second third ) or top level group of dependency
     * specs.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class AllDepSpec :
        public CompositeDepSpec,
        public Visitable<AllDepSpec, DepSpecVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            AllDepSpec();

            ///\}
    };

    /**
     * Represents a use? ( ) dependency spec.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class UseDepSpec :
        public CompositeDepSpec,
        public Visitable<UseDepSpec, DepSpecVisitorTypes>
    {
        private:
            const UseFlagName _flag;
            const bool _inverse;

        public:
            ///\name Basic operations
            ///\{

            UseDepSpec(const UseFlagName &, bool);

            ///\}

            /**
             * Fetch our use flag name.
             */
            UseFlagName flag() const;

            /**
             * Fetch whether we are a ! flag.
             */
            bool inverse() const;

            virtual const UseDepSpec * as_use_dep_spec() const;
    };

    /**
     * A StringDepSpec represents a non-composite dep spec with an associated
     * piece of text.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class StringDepSpec :
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

            void set_text(const std::string &);

        public:
            /**
             * Fetch our text.
             */
            std::string text() const;
    };

    /**
     * A selection of USE flag requirements.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class UseRequirements :
        private PrivateImplementationPattern<UseRequirements>
    {
        public:
            ///\name Basic operations
            ///\{

            UseRequirements();
            UseRequirements(const UseRequirements &);
            ~UseRequirements();

            ///\}

            ///\name Iterate over our USE requirements
            ///\{

            typedef libwrapiter::ForwardIterator<UseRequirements,
                    const std::pair<const UseFlagName, UseFlagState> > Iterator;

            Iterator begin() const;
            Iterator end() const;

            ///\}

            /// Find the requirement for a particular USE flag.
            Iterator find(const UseFlagName & u) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Insert a new requirement.
            bool insert(const UseFlagName & u, UseFlagState s)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /// What state is desired for a particular use flag?
            UseFlagState state(const UseFlagName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A PackageDepSpec represents a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class PackageDepSpec :
        public StringDepSpec,
        public Visitable<PackageDepSpec, DepSpecVisitorTypes>,
        private PrivateImplementationPattern<PackageDepSpec>
    {
        private:
            const PackageDepSpec & operator= (const PackageDepSpec &);

            void _do_parse(const std::string &, const PackageDepSpecParseMode);
            void _make_unique();

        public:
            ///\name Basic operations
            ///\{

            /**
             * Constructor, no version or SLOT restrictions.
             *
             * \deprecated Use the two arg form.
             */
            PackageDepSpec(const QualifiedPackageName & package) PALUDIS_ATTRIBUTE((deprecated));

            /**
             * Constructor, parse restrictions ourself.
             *
             * \deprecated Use the two arg form.
             */
            explicit PackageDepSpec(const std::string &) PALUDIS_ATTRIBUTE((deprecated));

            PackageDepSpec(const std::string &, const PackageDepSpecParseMode);

            explicit PackageDepSpec(
                std::tr1::shared_ptr<QualifiedPackageName> q = std::tr1::shared_ptr<QualifiedPackageName>(),
                std::tr1::shared_ptr<CategoryNamePart> c = std::tr1::shared_ptr<CategoryNamePart>(),
                std::tr1::shared_ptr<PackageNamePart> p = std::tr1::shared_ptr<PackageNamePart>(),
                std::tr1::shared_ptr<VersionRequirements> v = std::tr1::shared_ptr<VersionRequirements>(),
                VersionRequirementsMode m = vr_and,
                std::tr1::shared_ptr<SlotName> s = std::tr1::shared_ptr<SlotName>(),
                std::tr1::shared_ptr<RepositoryName> r = std::tr1::shared_ptr<RepositoryName>(),
                std::tr1::shared_ptr<UseRequirements> u = std::tr1::shared_ptr<UseRequirements>(),
                std::tr1::shared_ptr<const DepTag> t = std::tr1::shared_ptr<const DepTag>());

            PackageDepSpec(const PackageDepSpec &);

            ~PackageDepSpec();

            ///\}

            /**
             * Fetch the package name.
             */
            std::tr1::shared_ptr<const QualifiedPackageName> package_ptr() const;

            /**
             * Fetch the package name part, if wildcarded.
             */
            std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const;

            /**
             * Fetch the category name part, if wildcarded.
             */
            std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            std::tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const;

            /**
             * Fetch the version requirements (may be a zero pointer).
             */
            std::tr1::shared_ptr<VersionRequirements> version_requirements_ptr();

            /**
             * Fetch the version requirements mode.
             */
            VersionRequirementsMode version_requirements_mode() const;

            /**
             * Set the version requirements mode.
             */
            void set_version_requirements_mode(const VersionRequirementsMode m);

            /**
             * Fetch the slot name (may be a zero pointer).
             */
            std::tr1::shared_ptr<const SlotName> slot_ptr() const;

            /**
             * Fetch the repo name (may be a zero pointer).
             */
            std::tr1::shared_ptr<const RepositoryName> repository_ptr() const;

            /**
             * Fetch the use requirements (may be a zero pointer).
             */
            std::tr1::shared_ptr<const UseRequirements> use_requirements_ptr() const;

            /**
             * Fetch our tag.
             */
            std::tr1::shared_ptr<const DepTag> tag() const;

            /**
             * Set our tag.
             */
            void set_tag(const std::tr1::shared_ptr<const DepTag> & s);

            /**
             * Fetch a copy of ourself without the USE requirements.
             */
            std::tr1::shared_ptr<PackageDepSpec> without_use_requirements() const;

            virtual const PackageDepSpec * as_package_dep_spec() const;
    };

    /**
     * A PlainTextDepSpec represents a plain text entry (for example,
     * a URI in SRC_URI).
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class PlainTextDepSpec :
        public StringDepSpec,
        public Visitable<PlainTextDepSpec, DepSpecVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            PlainTextDepSpec(const std::string &);

            ///\}
    };

    /**
     * A PlainTextDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PlainTextDepSpec &);

    /**
     * Thrown if an invalid package dep spec specification is encountered.
     *
     * \ingroup grpexceptions
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class PackageDepSpecError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            PackageDepSpecError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * A PackageDepSpec can be written to an ostream.
     *
     * \ingroup grpdepspecs
     */
    std::ostream & operator<< (std::ostream &, const PackageDepSpec &);

    /**
     * A BlockDepSpec represents a block on a package name (for example,
     * 'app-editors/vim'), possibly with associated version and SLOT
     * restrictions.
     *
     * \ingroup grpdepspecs
     * \nosubgrouping
     */
    class BlockDepSpec :
        public StringDepSpec,
        public Visitable<BlockDepSpec, DepSpecVisitorTypes>
    {
        private:
            std::tr1::shared_ptr<const PackageDepSpec> _spec;

        public:
            ///\name Basic operations
            ///\{

            BlockDepSpec(std::tr1::shared_ptr<const PackageDepSpec> spec);

            ///\}

            /**
             * Fetch the spec we're blocking.
             */
            std::tr1::shared_ptr<const PackageDepSpec> blocked_spec() const;
    };

}

#endif
