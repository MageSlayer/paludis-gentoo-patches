/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_TAG_HH
#define PALUDIS_GUARD_PALUDIS_DEP_TAG_HH 1

/** \file
 * Declarations for dependency tags.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - \ref example_dep_tag.cc "example_dep_tag.cc" (for tags)
 * - \ref example_dep_spec.cc "example_dep_spec.cc" (for specifications)
 * - \ref example_dep_label.cc "example_dep_label.cc" (for labels)
 * - \ref example_dep_tree.cc "example_dep_tree.cc" (for specification trees)
 */

#include <paludis/dep_tag-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/operators.hh>

#include <string>
#include <paludis/util/tr1_memory.hh>

namespace paludis
{
    /**
     * Visitor class for visiting the different DepTag subclasses.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     * \since 0.26
     * \see DepTag
     */
    struct DepTagVisitorTypes :
        VisitorTypes<
            DepTagVisitorTypes,
            DepTag,
            GLSADepTag,
            GeneralSetDepTag,
            DependencyDepTag,
            TargetDepTag
        >
    {
    };

    /**
     * A DepTagCategory is identified by its name and has associated display
     * information for a DepTag's category.
     *
     * It is usually accessed via DepTagCategoryMaker.
     *
     * \see DepTagCategoryMaker
     * \see DepTag
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepTagCategory :
        private InstantiationPolicy<DepTagCategory, instantiation_method::NonCopyableTag>
    {
        private:
            bool _visible;
            const std::string _id;
            const std::string _title;
            const std::string _pre_text;
            const std::string _post_text;

        public:
            ///\name Basic operations
            ///\{

            DepTagCategory(
                    bool visible,
                    const std::string & id,
                    const std::string & t,
                    const std::string & pre,
                    const std::string & post);

            ///\}

            /**
             * Should we be displayed in a tag category summary?
             */
            bool visible() const;

            /**
             * Fetch our short ID (for example, 'GLSA').
             */
            std::string id() const;

            /**
             * Fetch our title (for example, 'Security advisories'), or an
             * empty string if we're untitled.
             */
            std::string title() const;

            /**
             * Fetch our pre list text, or an empty string.
             */
            std::string pre_text() const;

            /**
             * Fetch our post list text, or an empty string.
             */
            std::string post_text() const;
    };

    /**
     * Thrown if DepTagCategoryMaker cannot find the named DepTagCategory.
     *
     * \ingroup g_exceptions
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoSuchDepTagCategory :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            NoSuchDepTagCategory(const std::string &) throw ();

            ///\}
    };

    /**
     * Virtual constructor for accessing DepTagCategory instances.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepTagCategoryMaker :
        public VirtualConstructor<std::string, tr1::shared_ptr<const DepTagCategory> (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchDepTagCategory> >,
        public InstantiationPolicy<DepTagCategoryMaker, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<DepTagCategoryMaker, instantiation_method::SingletonTag>;

        private:
            DepTagCategoryMaker();
    };

    /**
     * A DepTag can be associated with a PackageDepSpec, and is transferred
     * onto any associated DepListEntry instances.
     *
     * It is used for tagging dep list entries visually, for example to
     * indicate an associated GLSA.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepTag :
        InstantiationPolicy<DepTag, instantiation_method::NonCopyableTag>,
        public relational_operators::HasRelationalOperators,
        public virtual ConstAcceptInterface<DepTagVisitorTypes>
    {
        protected:
            ///\name Basic operations
            ///\{

            DepTag();

        public:
            virtual ~DepTag();

            ///\}

            /**
             * Fetch our short text (for example, 'GLSA-1234') that is
             * displayed with the dep list entry.
             */
            virtual std::string short_text() const = 0;

            /**
             * Fetch our DepTagCategory's tag.
             */
            virtual std::string category() const = 0;

            ///\name Comparison operators
            ///\{

            bool operator< (const DepTag &) const;
            bool operator== (const DepTag &) const;

            ///\}
    };

    /**
     * DepTag subclass for GLSAs.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GLSADepTag :
        public DepTag,
        public ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GLSADepTag>
    {
        private:
            const std::string _id;
            const std::string _glsa_title;
            const FSEntry _glsa_file;

        public:
            ///\name Basic operations
            ///\{

            GLSADepTag(const std::string & id, const std::string & glsa_title, const FSEntry&);
            ~GLSADepTag();

            ///\}

            ///\name Content information
            ///\{

            virtual std::string short_text() const;

            virtual std::string category() const;

            /**
             * The full path to the glsa announcement file.
             */
            const FSEntry glsa_file() const;

            /**
             * Fetch our GLSA title (for example, 'Yet another PHP remote access
             * hole').
             */
            std::string glsa_title() const;

            ///\}
    };

    /**
     * DepTag subclass for general sets.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE GeneralSetDepTag :
        public DepTag,
        public ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, GeneralSetDepTag>,
        private PrivateImplementationPattern<GeneralSetDepTag>
    {
        public:
            ///\name Basic operations
            ///\{

            GeneralSetDepTag(const SetName & id, const std::string & source);
            ~GeneralSetDepTag();

            ///\}

            virtual std::string short_text() const;

            virtual std::string category() const;

            /**
             * From which repository or environment did we originate?
             */
            std::string source() const;
    };

    /**
     * DepTag subclass for dependencies.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DependencyDepTag :
        public DepTag,
        public ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, DependencyDepTag>,
        private PrivateImplementationPattern<DependencyDepTag>
    {
        private:
            void _make_str() const;

        public:
            ///\name Basic operations
            ///\{

            DependencyDepTag(const tr1::shared_ptr<const PackageID> &, const PackageDepSpec &,
                    const tr1::shared_ptr<const DependencySpecTree::ConstItem> &);

            ~DependencyDepTag();

            ///\}

            virtual std::string short_text() const;

            virtual std::string category() const;

            /**
             * The PackageID that contains our dependency.
             */
            const tr1::shared_ptr<const PackageID> package_id() const;

            /**
             * The PackageDepSpec that pulled us in.
             */
            const tr1::shared_ptr<const PackageDepSpec> dependency() const;

            /**
             * The AnyDepSpec instances and ConditionalDepSpec instances that our dependency
             * is conditional upon.
             */
            const tr1::shared_ptr<const DependencySpecTree::ConstItem> conditions() const;
    };

    /**
     * DepTag subclass for explicit targets.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE TargetDepTag :
        public DepTag,
        public ConstAcceptInterfaceVisitsThis<DepTagVisitorTypes, TargetDepTag>
    {
        public:
            ///\name Basic operations
            ///\{

            TargetDepTag();
            ~TargetDepTag();

            ///\}

            virtual std::string short_text() const;
            virtual std::string category() const;
    };

#include <paludis/dep_tag-sr.hh>

}

#endif
