/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UNINSTALL_TASK_HH
#define PALUDIS_GUARD_PALUDIS_UNINSTALL_TASK_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

/** \file
 * Declarations for UninstallTask.
 *
 * \ingroup g_tasks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;
    class UninstallListEntry;

    /**
     * Thrown if an ambiguous unmerge target is supplied.
     *
     * \ingroup g_exceptions
     * \ingroup g_tasks
     */
    class PALUDIS_VISIBLE AmbiguousUnmergeTargetError :
        public Exception
    {
        private:
            const std::string _t;
            const std::shared_ptr<const PackageIDSequence> _p;

        public:
            ///\name Basic operations
            ///\{

            AmbiguousUnmergeTargetError(const std::string & our_target,
                    const std::shared_ptr<const PackageIDSequence> matches) throw ();

            ~AmbiguousUnmergeTargetError() throw ();

            ///\}

            ///\name Iterate over our entries
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::shared_ptr<const PackageID> > ConstIterator;
            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}

            /**
             * What was our target?
             */
            std::string target() const;
    };

    /**
     * Task used to uninstall one or more targets.
     *
     * \ingroup g_tasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallTask
    {
        private:
            Pimp<UninstallTask> _imp;

        protected:
            ///\name Basic operations
            ///\{

            UninstallTask(Environment * const env);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~UninstallTask();

            UninstallTask(const UninstallTask &) = delete;
            UninstallTask & operator= (const UninstallTask &) = delete;

            ///\}

            ///\name Behaviour options
            ///\{

            void set_pretend(const bool value);
            void set_preserve_world(const bool value);
            void set_all_versions(const bool value);
            void set_with_unused_dependencies(const bool value);
            void set_with_dependencies(const bool value);
            void set_check_safety(const bool value);

            ///\}

            ///\name Add targets
            ///\{

            void add_target(const std::string &);
            void add_unused();

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_build_unmergelist_pre() = 0;
            virtual void on_build_unmergelist_post() = 0;

            virtual void on_display_unmerge_list_pre() = 0;
            virtual void on_display_unmerge_list_post() = 0;
            virtual void on_display_unmerge_list_entry(const UninstallListEntry &) = 0;

            virtual void on_uninstall_all_pre() = 0;
            virtual void on_uninstall_pre(const UninstallListEntry &) = 0;
            virtual void on_uninstall_post(const UninstallListEntry &) = 0;
            virtual void on_uninstall_all_post() = 0;

            virtual void on_not_continuing_due_to_errors() = 0;

            virtual void on_update_world_pre() = 0;
            virtual void on_update_world(const PackageDepSpec &) = 0;
            virtual void on_update_world(const SetName &) = 0;
            virtual void on_update_world_post() = 0;
            virtual void on_preserve_world() = 0;

            ///\}

            ///\name Logic
            ///\{

            virtual void world_remove_set(const SetName &);
            virtual void world_remove_packages(const std::shared_ptr<const SetSpecTree> &);

            ///\}

            /**
             * Run the task.
             */
            void execute();
    };
}

#endif
