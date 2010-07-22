/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_HOOKER_HH
#define PALUDIS_GUARD_PALUDIS_HOOKER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/graph-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <tr1/memory>
#include <string>

/** \file
 * Declarations for the Hooker class, which is used to run hooks.
 *
 * \ingroup g_hooks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class FSEntry;
    class Environment;
    class Hook;
    class HookResult;

    /**
     * A HookFile provides an abstraction of a hook file.
     *
     * \ingroup g_hooks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE HookFile
    {
        public:
            ///\name Basic operations
            ///\{

            HookFile() = default;
            virtual ~HookFile();

            HookFile(const HookFile &) = delete;
            HookFile & operator= (const HookFile &) = delete;

            ///\}

            virtual HookResult run(const Hook &) const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
            virtual const FSEntry file_name() const = 0;
            virtual void add_dependencies(const Hook &, DirectedGraph<std::string, int> &) = 0;
            virtual const std::tr1::shared_ptr<const Sequence<std::string> > auto_hook_names() const = 0;
    };

    /**
     * Handles executing hooks.
     *
     * \ingroup g_hooks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Hooker :
        private PrivateImplementationPattern<Hooker>
    {
        private:
            std::tr1::shared_ptr<Sequence<std::tr1::shared_ptr<HookFile> > > _find_hooks(const Hook &) const;

        public:
            ///\name Basic operations
            ///\{

            Hooker(const Environment * const) PALUDIS_ATTRIBUTE((nonnull(1)));
            ~Hooker();

            Hooker(const Hooker &) = delete;
            Hooker & operator= (const Hooker &) = delete;

            ///\}

            /**
             * Perform a hook, return HookResult.
             */
            HookResult perform_hook(const Hook &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Add a new hook directory.
             */
            void add_dir(const FSEntry &, const bool output_prefixed);
    };
}

#endif
