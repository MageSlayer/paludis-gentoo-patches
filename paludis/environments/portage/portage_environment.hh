/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PORTAGE_PORTAGE_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PORTAGE_PORTAGE_ENVIRONMENT_HH 1

#include <paludis/environment.hh>

namespace paludis
{
    class PortageEnvironmentConfigurationError :
        public ConfigurationError
    {
        public:
            PortageEnvironmentConfigurationError(const std::string &) throw ();
    };

    class PortageEnvironment :
        public Environment,
        private PrivateImplementationPattern<PortageEnvironment>
    {
        private:
            void _load_profile(const FSEntry &);
            void _add_virtuals_repository();
            void _add_installed_virtuals_repository();
            void _add_portdir_repository(const FSEntry &);
            void _add_portdir_overlay_repository(const FSEntry &);
            void _add_vdb_repository();

            template<typename I_>
            void _load_lined_file(const FSEntry &, I_);

            template<typename I_>
            void _load_atom_file(const FSEntry &, I_, const std::string &);

        public:
            PortageEnvironment(const std::string &);
            virtual ~PortageEnvironment();

            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void force_use(std::tr1::shared_ptr<const PackageDepSpec>, const UseFlagName &,
                    const UseFlagState);

            virtual void clear_forced_use();

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const,
                    const bool override_tilde_keywords = false) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(const UseFlagName &,
                    const PackageDatabaseEntry *) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool query_user_masks(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

#if 0
            virtual MirrorIterator begin_mirrors(const std::string & mirror) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual MirrorIterator end_mirrors(const std::string & mirror) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
#endif

            virtual FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual int perform_hook(const Hook & hook) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
