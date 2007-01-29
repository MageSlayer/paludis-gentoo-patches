/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
 * Declarations for the DepTag and DepTagCategory classes.
 *
 * \ingroup grpdeptag
 */

#include <paludis/package_database_entry.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/collection.hh>
#include <paludis/util/sr.hh>

#include <string>
#include <tr1/memory>

namespace paludis
{
    /**
     * A DepTagCategory is identified by its name and has associated display
     * information for a DepTag's category.
     *
     * It is usually accessed via DepTagCategoryMaker.
     *
     * \see DepTagCategoryMaker
     * \see DepTag
     *
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class DepTagCategory :
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
            bool visible() const
            {
                return _visible;
            }

            /**
             * Fetch our short ID (for example, 'GLSA').
             */
            std::string id() const
            {
                return _id;
            }

            /**
             * Fetch our title (for example, 'Security advisories'), or an
             * empty string if we're untitled.
             */
            std::string title() const
            {
                return _title;
            }

            /**
             * Fetch our pre list text, or an empty string.
             */
            std::string pre_text() const
            {
                return _pre_text;
            }

            /**
             * Fetch our post list text, or an empty string.
             */
            std::string post_text() const
            {
                return _post_text;
            }
    };

    /**
     * Thrown if DepTagCategoryMaker cannot find the named DepTagCategory.
     *
     * \ingroup grpexceptions
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class NoSuchDepTagCategory :
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
     * \ingroup grpdeptag
     */
    class DepTagCategoryMaker :
        public VirtualConstructor<std::string, std::tr1::shared_ptr<const DepTagCategory> (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchDepTagCategory> >,
        public InstantiationPolicy<DepTagCategoryMaker, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<DepTagCategoryMaker, instantiation_method::SingletonTag>;

        private:
            DepTagCategoryMaker();
    };

    class DepTag;
    class GLSADepTag;
    class GeneralSetDepTag;
    class DependencyDepTag;

    /**
     * Visitor class for visiting the different DepTag subclasses.
     *
     * \ingroup grpdeptag
     * \see DepTag
     */
    typedef VisitorTypes<GLSADepTag *, GeneralSetDepTag *, DependencyDepTag *> DepTagVisitorTypes;

    /**
     * A DepTag can be associated with a PackageDepAtom, and is transferred
     * onto any associated DepListEntry instances.
     *
     * It is used for tagging dep list entries visually, for example to
     * indicate an associated GLSA.
     *
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class DepTag :
        InstantiationPolicy<DepTag, instantiation_method::NonCopyableTag>,
        public virtual VisitableInterface<DepTagVisitorTypes>,
        public ComparisonPolicy<DepTag,
            comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberFetchFunctionTag<std::string> >
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
    };

    /**
     * DepTag subclass for GLSAs.
     *
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class GLSADepTag :
        public DepTag,
        public Visitable<GLSADepTag, DepTagVisitorTypes>
    {
        private:
            const std::string _id;
            const std::string _glsa_title;

        public:
            ///\name Basic operations
            ///\{

            GLSADepTag(const std::string & id, const std::string & glsa_title);

            ///\}

            virtual std::string short_text() const;

            virtual std::string category() const;

            /**
             * Fetch our GLSA title (for example, 'Yet another PHP remote access
             * hole').
             */
            std::string glsa_title() const;
    };

    /**
     * DepTag subclass for general sets.
     *
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class GeneralSetDepTag :
        public DepTag,
        public Visitable<GeneralSetDepTag, DepTagVisitorTypes>
    {
        private:
            const SetName _id;
            const std::string _source;

        public:
            ///\name Basic operations
            ///\{

            GeneralSetDepTag(const SetName & id, const std::string & source);

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
     * \ingroup grpdeptag
     * \nosubgrouping
     */
    class DependencyDepTag :
        public DepTag,
        public Visitable<DependencyDepTag, DepTagVisitorTypes>
    {
        private:
            const PackageDatabaseEntry _dbe;

        public:
            ///\name Basic operations
            ///\{

            DependencyDepTag(const PackageDatabaseEntry & dbe);

            ///\}

            virtual std::string short_text() const;

            virtual std::string category() const;
    };

#include <paludis/dep_tag-sr.hh>

    /**
     * Tags attached to a DepListEntry.
     *
     * \ingroup grpdeptag
     */
    typedef SortedCollection<DepTagEntry> DepListEntryTags;
}

#endif
