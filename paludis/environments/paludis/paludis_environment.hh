/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH 1

#include <paludis/environment_implementation.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>

/** \file
 * Declarations for the PaludisEnvironment class.
 *
 * \ingroup grppaludisenvironment
 */

namespace paludis
{
    namespace paludis_environment
    {
        class PaludisConfig;
    }

    /**
     * The PaludisEnvironment is an Environment that corresponds to the normal
     * operating evironment.
     *
     * \ingroup grppaludisenvironment
     */
    class PALUDIS_VISIBLE PaludisEnvironment :
        public EnvironmentImplementation
    {
        private:
            Pimp<PaludisEnvironment> _imp;

        protected:
            void need_keys_added() const override;
            void populate_sets() const override;

        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironment(const std::string &);

            ~PaludisEnvironment() override;

            ///\}

            ///\name PaludisEnvironment-specific information
            ///\{

            /**
             * The config directory.
             */
            std::string config_dir() const;

            ///\}

            std::shared_ptr<const FSPathSequence> bashrc_files() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const FSPathSequence> syncers_dirs() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const FSPathSequence> fetchers_dirs() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const FSPathSequence> hook_dirs() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string reduced_username() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            uid_t reduced_uid() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            gid_t reduced_gid() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            HookResult perform_hook(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::string distribution() const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool accept_keywords(const std::shared_ptr<const KeywordNameSet> &, const std::shared_ptr<const PackageID> &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool accept_license(const std::string &, const std::shared_ptr<const PackageID> &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::shared_ptr<const Mask> mask_for_user(const std::shared_ptr<const PackageID> &, const bool) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool unmasked_by_user(const std::shared_ptr<const PackageID> &, const std::string &) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            bool add_to_world(const QualifiedPackageName &) const override;

            bool add_to_world(const SetName &) const override;

            bool remove_from_world(const QualifiedPackageName &) const override;

            bool remove_from_world(const SetName &) const override;

            const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > config_location_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > preferred_root_key() const override;
            const std::shared_ptr<const MetadataValueKey<FSPath> > system_root_key() const override;

            const Tribool want_choice_enabled(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            const std::string value_for_choice_parameter(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &
                    ) const
                override PALUDIS_ATTRIBUTE((warn_unused_result));

            Tribool interest_in_suggestion(
                    const std::shared_ptr<const PackageID> & from_id,
                    const PackageDepSpec & spec) const override;

            const std::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const override;

            const std::shared_ptr<Repository> repository_from_new_config_file(
                    const FSPath &) override PALUDIS_ATTRIBUTE((warn_unused_result));

            void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const override;
    };
}
#endif
