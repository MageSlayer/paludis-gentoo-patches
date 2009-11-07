/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/choice-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/tribool.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/util/mutex-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/repositories/e/e_repository_id.hh>
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

    namespace n
    {
        struct arch_var_if_special;
        struct environment;
        struct location;
        struct mutex;
        struct profiles_explicitly_set;
        struct repository;
        struct repository_name;
        struct value;
    }

    class ERepositoryProfile;

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
                    const std::string & arch_var_if_special,
                    const bool profiles_explicitly_set);
            ~ERepositoryProfile();

            ///\}

            ///\name Use flag queries
            ///\{

            /// Is a use flag masked?
            bool use_masked(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName & value_unprefixed,
                    const ChoiceNameWithPrefix & value_prefixed
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Is a use flag forced?
            bool use_forced(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName & value_unprefixed,
                    const ChoiceNameWithPrefix & value_prefixed
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /// Use flag state, ignoring mask and force?
            Tribool use_state_ignoring_masks(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName & value_unprefixed,
                    const ChoiceNameWithPrefix & value_prefixed
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::tr1::shared_ptr<const erepository::ERepositoryID> &,
                    const std::tr1::shared_ptr<const Choice> &
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Iterate over USE_EXPAND, USE_EXPAND_HIDDEN etc
            ///\{

            const std::tr1::shared_ptr<const Set<std::string> > use_expand() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const Set<std::string> > use_expand_hidden() const PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::tr1::shared_ptr<const Set<std::string> > use_expand_unprefixed() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const Set<std::string> > use_expand_implicit() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const Set<std::string> > iuse_implicit() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::tr1::shared_ptr<const Set<std::string> > use_expand_values(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Environment variable queries
            ///\{

            /// What is the value of an environment variable?
            std::string environment_variable(const std::string &) const;

            ///\}

            ///\name Masks
            ///\{

            std::tr1::shared_ptr<const RepositoryMaskInfo> profile_masked(const PackageID &) const;

            ///\}

            ///\name System package set
            ///\{

            const std::tr1::shared_ptr<const SetSpecTree> system_packages() const;

            ///\}

            ///\name Virtuals
            ///\{

            struct VirtualsConstIteratorTag;
            typedef WrappedForwardIterator<VirtualsConstIteratorTag,
                const std::pair<const QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> > > VirtualsConstIterator;

            VirtualsConstIterator begin_virtuals() const;
            VirtualsConstIterator end_virtuals() const;
            VirtualsConstIterator find_virtual(const QualifiedPackageName &) const;

            ///\}
    };
}

#endif
