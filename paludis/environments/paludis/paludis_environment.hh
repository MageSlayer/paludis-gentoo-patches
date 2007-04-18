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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH 1

#include <paludis/package_database.hh>
#include <paludis/environment.hh>

/** \file
 * Declarations for the PaludisEnvironment class.
 *
 * \ingroup grppaludisenvironment
 */

namespace paludis
{
    class PaludisConfig;

    /**
     * The PaludisEnvironment is an Environment that corresponds to the normal
     * operating evironment.
     *
     * \ingroup grppaludisenvironment
     */
    class PALUDIS_VISIBLE PaludisEnvironment :
        public Environment,
        public InstantiationPolicy<PaludisEnvironment, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<PaludisEnvironment>
    {
        protected:
            std::tr1::shared_ptr<CompositeDepSpec> local_package_set(const SetName &) const;

        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironment(const std::string &);

            ~PaludisEnvironment();

            ///\}

            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const;

            virtual bool query_use(const UseFlagName &, const PackageDatabaseEntry *) const;

            virtual bool accept_keyword(const KeywordName &, const PackageDatabaseEntry * const,
                    const bool) const;

            virtual bool accept_license(const std::string &, const PackageDatabaseEntry * const) const;

            virtual bool query_user_masks(const PackageDatabaseEntry &) const;

            virtual bool query_user_unmasks(const PackageDatabaseEntry &) const;

            virtual std::string bashrc_files() const;

            virtual std::string hook_dirs() const;

            virtual std::string fetchers_dirs() const;

            virtual std::string syncers_dirs() const;

            virtual std::string paludis_command() const;

            virtual void set_paludis_command(const std::string &);

            virtual std::tr1::shared_ptr<const UseFlagNameCollection> known_use_expand_names(const UseFlagName &,
                    const PackageDatabaseEntry *) const;

            virtual int perform_hook(const Hook & hook) const PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual MirrorIterator begin_mirrors(const std::string & mirror) const;

            virtual MirrorIterator end_mirrors(const std::string & mirror) const;

            virtual FSEntry root() const;

            virtual uid_t reduced_uid() const;
            virtual gid_t reduced_gid() const;

            void force_use(std::tr1::shared_ptr<const PackageDepSpec>, const UseFlagName &,
                    const UseFlagState);

            virtual void clear_forced_use();

            virtual bool accept_breaks_portage() const;

            /**
             * The config directory.
             */
            std::string config_dir() const;
    };
}
#endif
