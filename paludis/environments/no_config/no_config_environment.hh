/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH 1

#include <paludis/environment_implementation.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/map-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_accept_unstable> accept_unstable;
        typedef Name<struct name_disable_metadata_cache> disable_metadata_cache;
        typedef Name<struct name_extra_accept_keywords> extra_accept_keywords;
        typedef Name<struct name_extra_params> extra_params;
        typedef Name<struct name_extra_repository_dirs> extra_repository_dirs;
        typedef Name<struct name_master_repository_name> master_repository_name;
        typedef Name<struct name_profiles_if_not_auto> profiles_if_not_auto;
        typedef Name<struct name_repository_dir> repository_dir;
        typedef Name<struct name_repository_type> repository_type;
        typedef Name<struct name_write_cache> write_cache;
    }

    namespace no_config_environment
    {
#include <paludis/environments/no_config/no_config_environment-se.hh>

        /**
         * Parameters for a NoConfigEnvironment.
         *
         * \see NoConfigEnvironment
         * \ingroup grpnoconfigenvironment
         * \nosubgrouping
         */
        struct Params
        {
            NamedValue<n::accept_unstable, bool> accept_unstable;
            NamedValue<n::disable_metadata_cache, bool> disable_metadata_cache;
            NamedValue<n::extra_accept_keywords, std::string> extra_accept_keywords;
            NamedValue<n::extra_params, std::shared_ptr<Map<std::string, std::string> > > extra_params;
            NamedValue<n::extra_repository_dirs, std::shared_ptr<const FSPathSequence> > extra_repository_dirs;
            NamedValue<n::master_repository_name, std::string> master_repository_name;

            /**
             * The profiles to use.
             *
             * Leave empty for automatic selection (which may not always be possible).
             *
             * \since 0.44
             */
            NamedValue<n::profiles_if_not_auto, std::string> profiles_if_not_auto;

            NamedValue<n::repository_dir, FSPath> repository_dir;
            NamedValue<n::repository_type, no_config_environment::RepositoryType> repository_type;
            NamedValue<n::write_cache, FSPath> write_cache;
        };
    }

    /**
     * An environment that uses a single repository, with no user configuration.
     *
     * \ingroup grpnoconfigenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoConfigEnvironment :
        public EnvironmentImplementation
    {
        private:
            Pimp<NoConfigEnvironment> _imp;

            virtual void need_keys_added() const;

        protected:
            virtual void populate_sets() const;

        public:
            ///\name Basic operations
            ///\{

            NoConfigEnvironment(const no_config_environment::Params & params);

            virtual ~NoConfigEnvironment();

            ///\}

            ///\name NoConfigEnvironment-specific configuration options
            ///\{

            /**
             * What is our top level directory for our main repository?
             */
            FSPath main_repository_dir() const;

            ///\}

            ///\name NoConfigEnvironment-specific repository information
            ///\{

            /**
             * Fetch our 'main' repository.
             */
            std::shared_ptr<Repository> main_repository();

            /**
             * Fetch our 'main' repository.
             */
            std::shared_ptr<const Repository> main_repository() const;

            /**
             * Fetch our 'master' repository (may be zero).
             */
            std::shared_ptr<Repository> master_repository();

            /**
             * Fetch our 'master' repository (may be zero).
             */
            std::shared_ptr<const Repository> master_repository() const;

            ///\}

            virtual const Tribool want_choice_enabled(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string value_for_choice_parameter(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual Tribool interest_in_suggestion(
                    const std::shared_ptr<const PackageID> & from_id,
                    const PackageDepSpec & spec) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(const std::shared_ptr<const KeywordNameSet> &, const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::shared_ptr<const Mask> mask_for_breakage(const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::shared_ptr<const Mask> mask_for_user(const std::shared_ptr<const PackageID> &, const bool will_be_used_for_overridden) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const std::shared_ptr<const PackageID> &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const FSPathSequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual std::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool add_to_world(const QualifiedPackageName &) const;

            virtual bool add_to_world(const SetName &) const;

            virtual bool remove_from_world(const QualifiedPackageName &) const;

            virtual bool remove_from_world(const SetName &) const;

            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > config_location_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > preferred_root_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > system_root_key() const;

            virtual const std::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const;

            virtual const std::shared_ptr<Repository> repository_from_new_config_file(
                    const FSPath &) PALUDIS_ATTRIBUTE((noreturn));

            virtual void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const;
    };
}

#endif
