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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PROFILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_PROFILE_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>
#include <string>

/** \file
 * Declaration for the ERepositoryProfile class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class ERepository;

    /**
     * Holds the profile data (but <em>not</em> the profiles/ top level data) for
     * a ERepository instance.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepositoryProfile :
        private PrivateImplementationPattern<ERepositoryProfile>,
        private InstantiationPolicy<ERepositoryProfile, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            ERepositoryProfile(const Environment * const env,
                    const ERepository * const,
                    const RepositoryName & name,
                    const FSEntrySequence & location,
                    const std::string & arch_var_if_special);
            ~ERepositoryProfile();

            ///\}

            ///\name Use flag queries
            ///\{

            /// Is a use flag masked?
            bool use_masked(const UseFlagName &, const PackageID &) const;

            /// Is a use flag forced?
            bool use_forced(const UseFlagName &, const PackageID &) const;

            /// Use flag state, ignoring mask and force?
            UseFlagState use_state_ignoring_masks(const UseFlagName &, const PackageID &) const;

            ///\}

            ///\name Iterate over USE_EXPAND, USE_EXPAND_HIDDEN
            ///\{

            typedef libwrapiter::ForwardIterator<ERepositoryProfile, const UseFlagName> UseExpandConstIterator;

            UseExpandConstIterator begin_use_expand() const;
            UseExpandConstIterator end_use_expand() const;
            UseExpandConstIterator begin_use_expand_hidden() const;
            UseExpandConstIterator end_use_expand_hidden() const;

            ///\}

            ///\name Environment variable queries
            ///\{

            /// What is the value of an environment variable?
            std::string environment_variable(const std::string &) const;

            ///\}

            ///\name Masks
            ///\{

            tr1::shared_ptr<const RepositoryMaskInfo> profile_masked(const PackageID &) const;

            ///\}

            ///\name System package set
            ///\{

            tr1::shared_ptr<SetSpecTree::ConstItem> system_packages() const;

            ///\}

            ///\name Virtuals
            ///\{

            typedef libwrapiter::ForwardIterator<ERepositoryProfile,
                const std::pair<const QualifiedPackageName, tr1::shared_ptr<const PackageDepSpec> > > VirtualsConstIterator;

            VirtualsConstIterator begin_virtuals() const;
            VirtualsConstIterator end_virtuals() const;
            VirtualsConstIterator find_virtual(const QualifiedPackageName &) const;

            ///\}
    };
}

#endif
