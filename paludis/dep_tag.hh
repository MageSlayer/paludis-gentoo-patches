/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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
 */

#include <paludis/dep_tag-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/operators.hh>
#include <paludis/util/type_list.hh>

#include <string>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct generation_name> generation;
        typedef Name<struct tag_name> tag;
    }

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
     * Factory for accessing DepTagCategory instances.
     *
     * \ingroup g_dep_spec
     * \since 0.30
     */
    class PALUDIS_VISIBLE DepTagCategoryFactory :
        public InstantiationPolicy<DepTagCategoryFactory, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<DepTagCategoryFactory, instantiation_method::SingletonTag>;

        private:
            DepTagCategoryFactory();

        public:
            const std::tr1::shared_ptr<DepTagCategory> create(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
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
        private InstantiationPolicy<DepTag, instantiation_method::NonCopyableTag>,
        public relational_operators::HasRelationalOperators,
        public virtual DeclareAbstractAcceptMethods<DepTag, MakeTypeList<
            GLSADepTag, GeneralSetDepTag, DependencyDepTag, TargetDepTag>::Type>
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
        public ImplementAcceptMethods<DepTag, GLSADepTag>
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
        public ImplementAcceptMethods<DepTag, GeneralSetDepTag>,
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
        public ImplementAcceptMethods<DepTag, DependencyDepTag>,
        private PrivateImplementationPattern<DependencyDepTag>
    {
        private:
            void _make_str() const;

        public:
            ///\name Basic operations
            ///\{

            DependencyDepTag(const std::tr1::shared_ptr<const PackageID> &, const PackageDepSpec &);

            ~DependencyDepTag();

            ///\}

            virtual std::string short_text() const;

            virtual std::string category() const;

            /**
             * The PackageID that contains our dependency.
             */
            const std::tr1::shared_ptr<const PackageID> package_id() const;

            /**
             * The PackageDepSpec that pulled us in.
             */
            const std::tr1::shared_ptr<const PackageDepSpec> dependency() const;
    };

    /**
     * DepTag subclass for explicit targets.
     *
     * \ingroup g_dep_spec
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE TargetDepTag :
        public DepTag,
        public ImplementAcceptMethods<DepTag, TargetDepTag>
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

    /**
     * Tags associated with a DepListEntry.
     *
     * The generation key is used internally by DepList. Its value is of no interest
     * to outside clients.
     *
     * \see DepListEntry
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct DepTagEntry
    {
        NamedValue<n::generation, long> generation;
        NamedValue<n::tag, std::tr1::shared_ptr<const DepTag> > tag;
    };

    /**
     * Compare two DepListEntry structs by tag only.
     *
     * \see DepTagEntry
     * \ingroup g_dep_list
     * \since 0.34
     */
    struct PALUDIS_VISIBLE DepTagEntryComparator
    {
        bool operator() (const DepTagEntry &, const DepTagEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class InstantiationPolicy<DepTagCategoryFactory, instantiation_method::SingletonTag>;
    extern template class PrivateImplementationPattern<DependencyDepTag>;
    extern template class PrivateImplementationPattern<GeneralSetDepTag>;
#endif

}

#endif
