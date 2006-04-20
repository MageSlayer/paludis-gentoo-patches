/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HH 1

#include <algorithm>
#include <bitset>
#include <deque>
#include <iterator>
#include <list>
#include <ostream>
#include <set>
#include <paludis/dep_atom.hh>
#include <paludis/dep_tag.hh>
#include <paludis/name.hh>
#include <paludis/qa/environment.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/smart_record.hh>
#include <paludis/version_spec.hh>

namespace paludis
{
    /**
     * Keys for a DepListEntry.
     */
    enum DepListEntryKeys
    {
        dle_name,            ///< Package name
        dle_version,         ///< Package version
        dle_metadata,        ///< Package SLOT
        dle_repository,      ///< Repository name
        dle_flags,           ///< Flags
        dle_tag,             ///< Our tag
        last_dle             ///< Number of entries
    };

    /**
     * Flags for a DepListEntry.
     */
    enum DepListEntryFlag
    {
        dlef_has_predeps,
        dlef_has_trypredeps,
        dlef_has_postdeps,
        dlef_skip,
        last_dlef
    };

    /**
     * Flags for a DepListEntry.
     */
    typedef std::bitset<last_dlef> DepListEntryFlags;

    /**
     * Tag for a DepListEntry.
     */
    struct DepListEntryTag :
        SmartRecordTag<comparison_mode::EqualityComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<DepListEntryKeys, last_dle>,
        SmartRecordKey<dle_name, QualifiedPackageName>,
        SmartRecordKey<dle_version, VersionSpec>,
        SmartRecordKey<dle_metadata, VersionMetadata::ConstPointer>,
        SmartRecordKey<dle_repository, RepositoryName>,
        SmartRecordKey<dle_flags, DepListEntryFlags>,
        SmartRecordKey<dle_tag, std::set<DepTag::ConstPointer, DepTag::Comparator> >
    {
    };

    /**
     * A DepListEntry represents an entry in a DepList.
     */
    typedef MakeSmartRecord<DepListEntryTag>::Type DepListEntry;

    /**
     * A DepListEntry can be written to a stream.
     */
    std::ostream & operator<< (std::ostream &, const DepListEntry &);

    /**
     * Thrown if an error occurs whilst building a DepList.
     */
    class DepListError : public Exception
    {
        protected:
            /**
             * Constructor.
             */
            DepListError(const std::string &) throw ();
    };

    /**
     * Thrown if a DepList's add stack gets too deep.
     */
    class DepListStackTooDeepError : public DepListError
    {
        public:
            /**
             * Constructor.
             */
            DepListStackTooDeepError(int level) throw ();
    };

    /**
     * Thrown if no entry in a || ( ) block is resolvable.
     */
    class NoResolvableOptionError : public DepListError
    {
        public:
            /**
             * Constructor.
             */
            NoResolvableOptionError() throw ();

            /**
             * Constructor with failure reasons.
             */
            template <typename I_>
            NoResolvableOptionError(I_ i, I_ end) throw ();
    };

    /**
     * Thrown if all versions of a particular atom are masked.
     */
    class AllMaskedError : public DepListError
    {
        private:
            std::string _query;

        public:
            /**
             * Constructor.
             */
            AllMaskedError(const std::string & query) throw ();

            /**
             * Destructor.
             */
            virtual ~AllMaskedError() throw ()
            {
            }

            /**
             * Our query.
             */
            const std::string & query() const
            {
                return _query;
            }
    };

    /**
     * Thrown if a block is encountered.
     */
    class BlockError : public DepListError
    {
        public:
            /**
             * Constructor.
             */
            BlockError(const std::string & msg) throw ();
    };

    /**
     * Thrown if a circular dependency is encountered.
     */
    class CircularDependencyError : public DepListError
    {
        private:
            unsigned _cycle_size;

        public:
            /**
             * Constructor, from a sequence of the items causing the circular
             * dependency.
             */
            template <typename I_>
            CircularDependencyError(I_ begin, const I_ end) throw () :
                DepListError("Circular dependency: " + join(begin, end, " -> ")),
                _cycle_size(std::distance(begin, end))
            {
            }

            /**
             * How large is our circular dependency cycle?
             */
            unsigned cycle_size() const
            {
                return _cycle_size;
            }
    };

    /**
     * Used in DepList::set_rdepend_post
     */
    enum DepListRdependOption 
    { 
        /**
         * RDEPENDs are always merged before the package; abort if this fails.
         */
        dlro_never,

        /**
         * RDEPENDs can be merged after the package, just before PDEPEND, if this is
         * necessary for correct resolution
         */
        dlro_as_needed,

        /**
         * RDEPENDs are always merged with PDEPENDs.
         */
        dlro_always
    };

    /**
     * Holds a list of dependencies in merge order.
     */
    class DepList :
        private InstantiationPolicy<DepList, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepList>,
        protected DepAtomVisitorTypes::ConstVisitor
    {
        private:
            void _add_raw(const DepAtom * const);

            void _add(DepAtom::ConstPointer a)
            {
                _add_raw(a.raw_pointer());
            }

            void _add_in_role_raw(const DepAtom * const, const std::string & role);

            void _add_in_role(DepAtom::ConstPointer a, const std::string & role)
            {
                _add_in_role_raw(a.raw_pointer(), role);
            }

        protected:
            ///\name Visit functions
            ///{
            void visit(const PlainTextDepAtom * const) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const PackageDepAtom * const);
            void visit(const UseDepAtom * const);
            void visit(const AnyDepAtom * const);
            void visit(const BlockDepAtom * const);
            void visit(const AllDepAtom * const);
            ///}

        public:
            /**
             * Iterator for our children.
             */
            typedef std::list<DepListEntry>::const_iterator Iterator;

            /**
             * Constructor.
             */
            DepList(const Environment * const);

            /**
             * Destructor.
             */
            virtual ~DepList();

            /**
             * Add the packages required to resolve an additional dependency
             * atom.
             */
            void add(DepAtom::ConstPointer);

            /**
             * Start of our dependency list.
             */
            Iterator begin() const;

            /**
             * One past the end of our dependency list.
             */
            Iterator end() const;

            /**
             * Behaviour: determines when RDEPEND entries can be treated as PDEPEND.
             */
            void set_rdepend_post(const DepListRdependOption value);

            /**
             * Behaviour: if set, a package that depends directly upon itself
             * will be accepted.
             */
            void set_drop_self_circular(const bool value);

            /**
             * Behaviour: if set, any circular dependencies are treated as if
             * they do not exist.
             */
            void set_drop_circular(const bool value);

            /**
             * Behaviour: if set, any dependencies are treated as if
             * they do not exist.
             */
            void set_drop_all(const bool value);

            /**
             * Behaviour: ignore installed packages.
             */
            void set_ignore_installed(const bool value);

            /**
             * Behaviour: check nth level dependencies for packages that are
             * already installed.
             */
            void set_recursive_deps(const bool value);

            /**
             * Behaviour: set the maximum stack depth.
             */
            void set_max_stack_depth(const int value);

            /**
             * Behaviour: set whether we reinstall first level deps.
             */
            void set_reinstall(const bool value);
    };
}

#endif
