/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <string>
#include <paludis/package_database_entry.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/collection.hh>
#include <paludis/util/sr.hh>

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
     */
    class DepTagCategory :
        InstantiationPolicy<DepTagCategory, instantiation_method::NonCopyableTag>,
        public InternalCounted<DepTagCategory>
    {
        private:
            bool _visible;
            const std::string _id;
            const std::string _title;
            const std::string _pre_text;
            const std::string _post_text;

        public:
            /**
             * Constructor.
             */
            DepTagCategory(
                    bool visible,
                    const std::string & id,
                    const std::string & t,
                    const std::string & pre,
                    const std::string & post);

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
     */
    class NoSuchDepTagCategory :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            NoSuchDepTagCategory(const std::string &) throw ();
    };

    /**
     * Virtual constructor for accessing DepTagCategory instances.
     *
     * \ingroup grpdeptag
     */
    typedef VirtualConstructor<std::string, DepTagCategory::ConstPointer (*) (),
            virtual_constructor_not_found::ThrowException<NoSuchDepTagCategory> > DepTagCategoryMaker;

    class DepTag;
    class GLSADepTag;
    class GeneralSetDepTag;
    class DependencyDepTag;

    /**
     * Visitor class for visiting the different DepTag subclasses.
     *
     * \ingroup grpdeptag
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
     */
    class DepTag :
        InstantiationPolicy<DepTag, instantiation_method::NonCopyableTag>,
        public InternalCounted<DepTag>,
        public virtual VisitableInterface<DepTagVisitorTypes>,
        public ComparisonPolicy<DepTag,
            comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberFetchFunctionTag<std::string> >
    {
        protected:
            /**
             * Constructor.
             */
            DepTag();

        public:
            /**
             * Destructor.
             */
            virtual ~DepTag();

            /**
             * Fetch our short text (for example, 'GLSA-1234') that is
             * displayed with the dep list entry.
             */
            virtual std::string short_text() const = 0;

            /**
             * Fetch our DepTagCategory's tag.
             */
            virtual std::string category() const = 0;

            /**
             * Compare, by short_text only.
             */
    };

    /**
     * DepTag subclass for GLSAs.
     *
     * \ingroup grpdeptag
     */
    class GLSADepTag :
        public DepTag,
        public Visitable<GLSADepTag, DepTagVisitorTypes>
    {
        private:
            const std::string _id;
            const std::string _glsa_title;

        public:
            /**
             * Constructor.
             */
            GLSADepTag(const std::string & id, const std::string & glsa_title);

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
     */
    class GeneralSetDepTag :
        public DepTag,
        public Visitable<GeneralSetDepTag, DepTagVisitorTypes>
    {
        private:
            const std::string _id;

        public:
            /**
             * Constructor.
             */
            GeneralSetDepTag(const std::string & id);

            virtual std::string short_text() const;

            virtual std::string category() const;
    };

    /**
     * DepTag subclass for dependencies.
     *
     * \ingroup grpdeptag
     */
    class DependencyDepTag :
        public DepTag,
        public Visitable<DependencyDepTag, DepTagVisitorTypes>
    {
        private:
            const PackageDatabaseEntry _dbe;

        public:
            /**
             * Constructor.
             */
            DependencyDepTag(const PackageDatabaseEntry & dbe);

            virtual std::string short_text() const;

            virtual std::string category() const;
    };

#include <paludis/dep_tag-sr.hh>

    typedef SortedCollection<DepTagEntry> DepListEntryTags;
}

#endif
