/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/virtual_constructor.hh>
#include <paludis/util/exception.hh>

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
            const std::string _id;
            const std::string _title;
            const std::string _pre_text;
            const std::string _post_text;

        public:
            /**
             * Constructor.
             */
            DepTagCategory(const std::string & id,
                    const std::string & t,
                    const std::string & pre,
                    const std::string & post);

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

    /**
     * Visitor class for visiting the different DepTag subclasses.
     *
     * \ingroup grpdeptag
     */
    typedef VisitorTypes<GLSADepTag *> DepTagVisitorTypes;

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
        public virtual VisitableInterface<DepTagVisitorTypes>
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
             * Used for comparisons in containers containing pointers to DepTag
             * instances.
             *
             * \ingroup grpdeptag
             */
            struct Comparator
            {
                /// Perform the comparison.
                bool operator() (const DepTag::ConstPointer & d1,
                        const DepTag::ConstPointer & d2) const
                {
                    return d1->short_text() < d2->short_text();
                }
            };
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
}

#endif
