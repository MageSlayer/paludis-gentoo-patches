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

#ifndef PALUDIS_GUARD_SRC_OUTPUT_CONSOLE_QUERY_TASK_HH
#define PALUDIS_GUARD_SRC_OUTPUT_CONSOLE_QUERY_TASK_HH 1

#include <paludis/environment.hh>
#include <paludis/dep_spec.hh>
#include <src/output/console_task.hh>

namespace paludis
{
    class PALUDIS_VISIBLE ConsoleQueryTask :
        private PrivateImplementationPattern<ConsoleQueryTask>,
        public ConsoleTask
    {
        protected:
            ConsoleQueryTask(const Environment * const env);

        public:
            virtual ~ConsoleQueryTask();

            virtual void show(const PackageDepSpec &, const PackageDatabaseEntry * = 0) const;

            virtual void display_header(const PackageDepSpec &, const PackageDatabaseEntry &) const;
            virtual void display_versions_by_repository(const PackageDepSpec &,
                    tr1::shared_ptr<const PackageDatabaseEntryCollection>, const PackageDatabaseEntry &) const;
            virtual void display_metadata(const PackageDepSpec &, const PackageDatabaseEntry &) const;

            virtual void display_metadata_key(const std::string &, const std::string &,
                    const std::string &) const;
            virtual void display_metadata_license(const std::string &, const std::string &,
                    tr1::shared_ptr<const DepSpec>, const PackageDatabaseEntry &) const;
            virtual void display_metadata_dep(const std::string &, const std::string &,
                    tr1::shared_ptr<const DepSpec>, const bool one_line) const;
            virtual void display_metadata_pde(const std::string &, const std::string &, const PackageDatabaseEntry &) const;
            virtual void display_metadata_time(const std::string &, const std::string &, time_t) const;
            virtual void display_metadata_iuse(const std::string &, const std::string &, const std::string &,
                    const PackageDatabaseEntry &) const;

            virtual bool want_deps() const = 0;
            virtual bool want_raw() const = 0;

            const MaskReasons mask_reasons_to_explain() const;
    };
}

#endif
