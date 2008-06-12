/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_ADAPTED_ADAPTED_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_ADAPTED_ADAPTED_ENVIRONMENT_HH 1

#include <paludis/environment.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class PackageDepSpec;

    /**
     * An Environment that allows you to change aspects of an
     * existing Environment, e.g.\ the state of a USE flag for a
     * package.
     *
     * \ingroup grpadaptedenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AdaptedEnvironment :
        public Environment,
        private PrivateImplementationPattern<AdaptedEnvironment>
    {
        private:
            PrivateImplementationPattern<AdaptedEnvironment>::ImpPtr & _imp;

        protected:
            virtual void clear_metadata_keys() const;
            virtual void need_keys_added() const;

        public:
            AdaptedEnvironment(std::tr1::shared_ptr<Environment>);
            ~AdaptedEnvironment();

            ///\name Adapting methods
            ///\{

            /**
             * Set the state of a USE flag for the given PackageDepSpec.
             */
            void adapt_use(std::tr1::shared_ptr<const PackageDepSpec>, const UseFlagName &, const UseFlagState);

            /**
             * Clear all adaptions from this Environemnt.
             */
            void clear_adaptions();

            ///\}

            virtual std::tr1::shared_ptr<PackageIDSequence> operator[] (const Selection &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const UseFlagNameSet> known_use_expand_names(
                    const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(std::tr1::shared_ptr<const KeywordNameSet>, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            virtual const FSEntry root() const;

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual std::tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const SetNameSet> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<SetSpecTree::ConstItem> set(const SetName & id) const;

            virtual std::tr1::shared_ptr<const DestinationsSet> default_destinations() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool is_paludis_package(const QualifiedPackageName &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const SetName &) const;
    };
}

#endif
