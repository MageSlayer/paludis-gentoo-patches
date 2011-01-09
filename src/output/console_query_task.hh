/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2011 Ciaran McCreesh
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

#include <paludis/environment-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/metadata_key-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/map-fwd.hh>
#include <src/output/console_task.hh>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE ConsoleQueryTask :
        public ConsoleTask
    {
        private:
            Pimp<ConsoleQueryTask> _imp;

        protected:
            ConsoleQueryTask(const Environment * const env);

            virtual void show_one(const PackageDepSpec &, const std::shared_ptr<const PackageID> & = std::shared_ptr<const PackageID>()) const;

        public:
            virtual ~ConsoleQueryTask();

            virtual void show(const PackageDepSpec &, const std::shared_ptr<const PackageID> & = std::shared_ptr<const PackageID>()) const;

            virtual void display_header(const PackageDepSpec &, const std::shared_ptr<const PackageID> &) const;
            virtual void display_versions_by_repository(const PackageDepSpec &,
                    const std::shared_ptr<const PackageIDSequence> &, const std::shared_ptr<const PackageID> &) const;
            virtual void display_metadata(const PackageDepSpec &, const std::shared_ptr<const PackageID> &) const;

            virtual void display_metadata_key(const std::string &, const std::string &,
                    const std::string &) const;

            virtual void display_masks(const PackageDepSpec &, const std::shared_ptr<const PackageID> &) const;

            virtual void display_compact(const PackageDepSpec &, const std::shared_ptr<const PackageID> &) const;

            virtual bool want_compact() const = 0;
            virtual bool want_deps() const = 0;
            virtual bool want_raw() const = 0;
            virtual bool want_authors() const = 0;

            const std::shared_ptr<const Map<char, std::string> > masks_to_explain() const;
    };
}

#endif
