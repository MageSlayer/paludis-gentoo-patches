/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/contents-fwd.hh>

#include <paludis/util/visitor.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <tr1/memory>

#include <string>

/** \file
 * Declarations for the Contents classes.
 *
 * \ingroup g_contents
 *
 * \section Examples
 *
 * - \ref example_contents.cc "example_contents.cc"
 */

namespace paludis
{
    /**
     * Visit a contents heirarchy.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    struct ContentsVisitorTypes :
        VisitorTypes<
            ContentsVisitorTypes,
            ContentsEntry,
            ContentsFileEntry,
            ContentsDirEntry,
            ContentsSymEntry,
            ContentsFifoEntry,
            ContentsDevEntry,
            ContentsMiscEntry
        >
    {
    };

    /**
     * Base class for a contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsEntry :
        private InstantiationPolicy<ContentsEntry, instantiation_method::NonCopyableTag>,
        public virtual ConstAcceptInterface<ContentsVisitorTypes>
    {
        friend std::ostream & operator<< (std::ostream &, const ContentsEntry &);

        private:
            std::string _name;

        protected:
            ///\name Basic operations
            ///\{

            ContentsEntry(const std::string & our_name);

            /**
             * Used to implement the ostream operator<<.
             */
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~ContentsEntry();

            ///\}

            /// Our name.
            std::string name() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A file contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsFileEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsFileEntry>
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
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsDirEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsDirEntry>
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
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsMiscEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsMiscEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsMiscEntry(const std::string & name);

            ///\}
    };

    /**
     * A fifo contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsFifoEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsFifoEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsFifoEntry(const std::string & name);

            ///\}
    };

    /**
     * A device contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsDevEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsDevEntry>
    {
        public:
            ///\name Basic operations
            ///\{

            ContentsDevEntry(const std::string & name);

            ///\}
    };

    /**
     * A sym contents entry.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ContentsSymEntry :
        public ContentsEntry,
        public ConstAcceptInterfaceVisitsThis<ContentsVisitorTypes, ContentsSymEntry>
    {
        private:
            std::string _target;

        protected:
            virtual const std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            ContentsSymEntry(const std::string & name, const std::string & target);

            ///\}

            /// Our target (as per readlink).
            std::string target() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A package's contents, obtainable by PackageID::contents_key.
     *
     * \ingroup g_contents
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Contents :
        private InstantiationPolicy<Contents, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<Contents>
    {
        public:
            ///\name Basic operations
            ///\{

            Contents();
            ~Contents();

            ///\}

            /// Add a new entry.
            void add(std::tr1::shared_ptr<const ContentsEntry> c);

            ///\name Iterate over our entries
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const ContentsEntry> > ConstIterator;

            ConstIterator begin() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ConstIterator end() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };
}

#endif
