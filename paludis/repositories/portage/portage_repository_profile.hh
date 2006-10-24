/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_PROFILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_PROFILE_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <string>

/** \file
 * Declaration for the PortageRepositoryProfile class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class Environment;

    /**
     * Holds the profile data (but <em>not</em> the profiles/ top level data) for
     * a PortageRepository instance.
     *
     * \ingroup grpportagerepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PortageRepositoryProfile :
        private PrivateImplementationPattern<PortageRepositoryProfile>,
        private InstantiationPolicy<PortageRepositoryProfile, instantiation_method::NonCopyableTag>,
        public InternalCounted<PortageRepositoryProfile>
    {
        public:
            ///\name Basic operations
            ///\{

            PortageRepositoryProfile(const Environment * const env,
                    const RepositoryName & name,
                    const FSEntryCollection & location);
            ~PortageRepositoryProfile();

            ///\}

            ///\name Use flag queries
            ///\{

            /// Is a use flag masked?
            bool use_masked(const UseFlagName &, const PackageDatabaseEntry * const) const;

            /// Is a use flag forced?
            bool use_forced(const UseFlagName &, const PackageDatabaseEntry * const) const;

            /// Use flag state, ignoring mask and force?
            UseFlagState use_state_ignoring_masks(const UseFlagName &) const;

            ///\}

            ///\name Iterate over USE_EXPAND, USE_EXPAND_HIDDEN
            ///\{

            typedef libwrapiter::ForwardIterator<PortageRepositoryProfile, const UseFlagName> UseExpandIterator;

            UseExpandIterator begin_use_expand() const;
            UseExpandIterator end_use_expand() const;
            UseExpandIterator begin_use_expand_hidden() const;
            UseExpandIterator end_use_expand_hidden() const;

            ///\}

            ///\name Environment variable queries
            ///\{

            /// What is the value of an environment variable?
            std::string environment_variable(const std::string &) const;

            ///\}

            ///\name Masks
            ///\{

            bool profile_masked(const QualifiedPackageName &, const VersionSpec &,
                    const RepositoryName &) const;

            ///\}

            ///\name System package set
            ///\{

            AllDepAtom::Pointer system_packages() const;

            ///\}

            ///\name Virtuals
            ///\{

            typedef libwrapiter::ForwardIterator<PortageRepositoryProfile,
                const std::pair<const QualifiedPackageName, PackageDepAtom::ConstPointer> > VirtualsIterator;

            VirtualsIterator begin_virtuals() const;
            VirtualsIterator end_virtuals() const;
            VirtualsIterator find_virtual() const;

            ///\}
    };
}

#endif
