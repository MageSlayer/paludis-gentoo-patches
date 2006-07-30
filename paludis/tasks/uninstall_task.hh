/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_UNINSTALL_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_UNINSTALL_TASK_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    class Environment;

    class AmbiguousUnmergeTargetError :
        public Exception
    {
        private:
            const std::string _t;
            const PackageDatabaseEntryCollection::ConstPointer _p;

        public:
            AmbiguousUnmergeTargetError(const std::string & target,
                    const PackageDatabaseEntryCollection::ConstPointer matches) throw () :
                Exception("Ambiguous unmerge target '" + target + "'"),
                _t(target),
                _p(matches)
            {
            }

            ~AmbiguousUnmergeTargetError() throw ();

            typedef PackageDatabaseEntryCollection::Iterator Iterator;

            Iterator begin() const
            {
                return _p->begin();
            }

            Iterator end() const
            {
                return _p->end();
            }

            const std::string & target() const
            {
                return _t;
            }
    };

    class UninstallTask :
        PrivateImplementationPattern<UninstallTask>,
        InstantiationPolicy<UninstallTask, instantiation_method::NonCopyableTag>
    {
        protected:
            ///\name Basic operations
            ///\{

            UninstallTask(Environment * const env);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~UninstallTask();

            ///\}

            ///\name Behaviour options
            ///\{

            void set_no_config_protect(const bool value);
            void set_pretend(const bool value);
            void set_preserve_world(const bool value);

            ///\}

            ///\name Add targets
            ///\{

            void add_target(const std::string &);

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_build_unmergelist_pre() = 0;
            virtual void on_build_unmergelist_post() = 0;

            virtual void on_display_unmerge_list_pre() = 0;
            virtual void on_display_unmerge_list_post() = 0;
            virtual void on_display_unmerge_list_entry(const PackageDatabaseEntry &) = 0;

            virtual void on_uninstall_all_pre() = 0;
            virtual void on_uninstall_pre(const PackageDatabaseEntry &) = 0;
            virtual void on_uninstall_post(const PackageDatabaseEntry &) = 0;
            virtual void on_uninstall_all_post() = 0;

            virtual void on_update_world_pre() = 0;
            virtual void on_update_world(const PackageDepAtom &) = 0;
            virtual void on_update_world_post() = 0;
            virtual void on_preserve_world() = 0;

            ///\}

            /**
             * Run the task.
             */
            void execute();
    };
}

#endif
