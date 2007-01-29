/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_STAGE_BUILDER_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_STAGE_BUILDER_TASK_HH 1

#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>

#include <tr1/memory>

namespace paludis
{
    /**
     * Thrown if a stage build fails.
     *
     * \ingroup grpstagebuildertask
     * \nosubgrouping
     */
    class StageBuildError :
        public Exception
    {
        public:
            /**
             * Constructor
             *
             * \param message Message associated with the build error.
             */
            StageBuildError(const std::string & message) throw ();
    };

    #include <paludis/tasks/stage_options-sr.hh>

    /**
     * Represents the base class for all stages which can be build by
     * StageBuilderTask.
     *
     * \ingroup grptask
     * \ingroup grpstagebuildertask
     * \nosubgrouping
     */
    class StageBase
    {
        public:
            virtual ~StageBase();

            /// Build the stage.
            virtual int build(const StageOptions &) const = 0;

            /// Verbose description for this stage.
            virtual std::string description() const = 0;

            /// Has this stage ever been built before?
            virtual bool is_rebuild() const = 0;

            /// Short name for this stage.
            virtual std::string short_name() const = 0;
    };

    /**
     * Task to handle building for of one or more descendants of StageBase.
     *
     * \ingroup grptasks
     * \ingroup grpstagehandlertask
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE StageBuilderTask :
        PrivateImplementationPattern<StageBuilderTask>,
        InstantiationPolicy<StageBuilderTask, instantiation_method::NonCopyableTag>
    {
        protected:
            ///\name Basic operations
            ///\{

            StageBuilderTask(const StageOptions &);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~StageBuilderTask();

            ///\}

            ///\name Queue stage in build list
            ///\{

            void queue_stage(std::tr1::shared_ptr<const StageBase>);

            ///\}

            ///\name Iterate over our stages
            ///\{

            typedef libwrapiter::ForwardIterator<StageBuilderTask, const std::tr1::shared_ptr<const StageBase> > StageIterator;
            StageIterator begin_stages() const;
            StageIterator end_stages() const;

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_build_all_pre() = 0;
            virtual void on_build_pre(std::tr1::shared_ptr<const StageBase>) = 0;
            virtual void on_build_post(std::tr1::shared_ptr<const StageBase>) = 0;
            virtual void on_build_fail(std::tr1::shared_ptr<const StageBase>, const StageBuildError &) = 0;
            virtual void on_build_skipped(std::tr1::shared_ptr<const StageBase>) = 0;
            virtual void on_build_succeed(std::tr1::shared_ptr<const StageBase>) = 0;
            virtual void on_build_all_post() = 0;

            ///\}

            /**
             * Run the task
             */

            void execute();
    };
}

#endif
