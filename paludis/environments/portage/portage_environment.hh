/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_E_E_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_E_E_ENVIRONMENT_HH 1

#include <paludis/environment_implementation.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace portage_environment
    {
        /**
         * Thrown if a configuration error occurs in a PortageEnvironment.
         *
         * \ingroup grpportageenvironment
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE PortageEnvironmentConfigurationError :
            public ConfigurationError
        {
            public:
                ///\name Basic operations
                ///\{

                PortageEnvironmentConfigurationError(const std::string &) throw ();

                ///\}
        };
    }

    /**
     * Environment using Portage-like configuration files.
     *
     * \ingroup grpportageenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PortageEnvironment :
        public EnvironmentImplementation,
        private PrivateImplementationPattern<PortageEnvironment>
    {
        private:
            PrivateImplementationPattern<PortageEnvironment>::ImpPtr & _imp;

            void _load_profile(const FSEntry &);
            void _add_virtuals_repository();
            void _add_installed_virtuals_repository();
            void _add_portdir_repository(const FSEntry &);
            void _add_portdir_overlay_repository(const FSEntry &);
            void _add_ebuild_repository(const FSEntry &, const std::string &,
                    const std::string &, int importance);
            void _add_vdb_repository();

            template<typename I_>
            void _load_lined_file(const FSEntry &, I_);

            template<typename I_>
            void _load_atom_file(const FSEntry &, I_, const std::string &, const bool);

            void _add_string_to_world(const std::string &) const;
            void _remove_string_from_world(const std::string &) const;

        protected:
            virtual void need_keys_added() const;
            virtual void populate_sets() const;

        public:
            ///\name Basic operations
            ///\{

            PortageEnvironment(const std::string &);
            virtual ~PortageEnvironment();

            ///\}

            virtual const Tribool want_choice_enabled(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string value_for_choice_parameter(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<const Choice> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID &, const bool) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const SetName &) const;

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > config_location_key() const;

            virtual const std::tr1::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const;

            virtual const std::tr1::shared_ptr<Repository> repository_from_new_config_file(
                    const FSEntry &) PALUDIS_ATTRIBUTE((noreturn));

            virtual void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const;
    };
}

#endif
