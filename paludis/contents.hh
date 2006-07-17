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

#ifndef PALUDIS_GUARD_PALUDIS_CONTENTS_HH
#define PALUDIS_GUARD_PALUDIS_CONTENTS_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

#include <libwrapiter/libwrapiter.hh>

/** \file
 * Declarations for the Contents classes.
 *
 * \ingroup grpcontents
 */

namespace paludis
{
    struct ContentsEntry;
    struct ContentsFileEntry;
    struct ContentsDirEntry;
    struct ContentsSymEntry;
    struct ContentsMiscEntry;

    /**
     * Visit a contents heirarchy.
     *
     * \ingroup grpcontents
     */
    typedef VisitorTypes<ContentsFileEntry *, ContentsDirEntry *,
            ContentsSymEntry *, ContentsMiscEntry *> ContentsVisitorTypes;

    /**
     * Base class for a contents entry.
     *
     * \ingroup grpcontents
     */
    class ContentsEntry :
        private InstantiationPolicy<ContentsEntry, instantiation_method::NonCopyableTag>,
        public InternalCounted<ContentsEntry>,
        public virtual VisitableInterface<ContentsVisitorTypes>
    {
        private:
            std::string _name;

        protected:
            ///\name Basic operations
            ///\{

            ContentsEntry(const std::string & name) :
                _name(name)
            {
            }

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~ContentsEntry();

            ///\}

            /// Our name.
            std::string name() const
            {
                return _name;
            }
    };

    /**
     * A file contents entry.
     *
     * \ingroup grpcontents
     */
    class ContentsFileEntry :
        public ContentsEntry,
        public Visitable<ContentsFileEntry, ContentsVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsFileEntry(const std::string & name);

            ///\}
    };

    /**
     * A directory contents entry.
     *
     * \ingroup grpcontents
     */
    class ContentsDirEntry :
        public ContentsEntry,
        public Visitable<ContentsDirEntry, ContentsVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsDirEntry(const std::string & name);

            ///\}
    };

    /**
     * A misc contents entry.
     *
     * \ingroup grpcontents
     */
    class ContentsMiscEntry :
        public ContentsEntry,
        public Visitable<ContentsMiscEntry, ContentsVisitorTypes>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsMiscEntry(const std::string & name);

            ///\}
    };

    /**
     * A sym contents entry.
     *
     * \ingroup grpcontents
     */
    class ContentsSymEntry :
        public ContentsEntry,
        public Visitable<ContentsSymEntry, ContentsVisitorTypes>
    {
        private:
            std::string _target;

        public:
            ///\name Basic operations
            ///\{

            ContentsSymEntry(const std::string & name, const std::string & target);

            ///\}

            /// Our target (as per readlink).
            std::string target() const
            {
                return _target;
            }
    };

    /**
     * A package's contents.
     *
     * \ingroup grpcontents
     */
    class Contents :
        private InstantiationPolicy<Contents, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<Contents>,
        public InternalCounted<Contents>
    {
        public:
            ///\name Basic operations
            ///\{

            Contents();
            ~Contents();

            ///\}

            /// Add a new entry.
            void add(ContentsEntry::ConstPointer c);

            ///\name Iterate over our entries
            ///\{

            typedef libwrapiter::ForwardIterator<Contents, const ContentsEntry::ConstPointer> Iterator;

            Iterator begin() const;

            Iterator end() const;

            ///\}
    };
}

#endif
