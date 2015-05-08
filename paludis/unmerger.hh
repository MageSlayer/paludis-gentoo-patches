/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/named_value.hh>
#include <paludis/merger_entry_type.hh>
#include <paludis/contents-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <functional>

/** \file
 * Declarations for the Unmerger class, which can be used by Repository
 * to implement from-filesystem unmerging.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Hook;
    class Environment;

    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_ignore> ignore;
        typedef Name<struct name_maybe_output_manager> maybe_output_manager;
        typedef Name<struct name_root> root;
    }

    /**
     * Options for a basic Unmerger.
     *
     * \see Unmerger
     * \ingroup g_repository
     * \since 0.30
     */
    struct UnmergerOptions
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::ignore, const std::function<bool (const FSPath &)> > ignore;
        NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
        NamedValue<n::root, FSPath> root;
    };

    /**
     * Thrown if an error occurs during an unmerge.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UnmergerError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            UnmergerError(const std::string & msg) noexcept;

            ///\}
    };

    /**
     * Handles unmerging items.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Unmerger
    {
        private:
            Pimp<Unmerger> _imp;

        protected:
            ///\name Basic operations
            ///\{

            Unmerger(const UnmergerOptions &);

            ///\}

            /**
             * Add entry to the unmerge set.
             */
            void add_unmerge_entry(const EntryType, const std::shared_ptr<const ContentsEntry> &);

            /**
             * Populate the unmerge set.
             */
            virtual void populate_unmerge_set() = 0;

            /**
             * Extend a hook with extra options.
             */
            virtual Hook extend_hook(const Hook &) const;

            ///\name Unmerge operations
            ///\{

            virtual void unmerge_file(const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unmerge_dir(const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unmerge_sym(const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unmerge_misc(const std::shared_ptr<const ContentsEntry> &) const;

            ///\}

            ///\name Check operations
            ///\{

            virtual bool check_file(const std::shared_ptr<const ContentsEntry> &) const;
            virtual bool check_dir(const std::shared_ptr<const ContentsEntry> &) const;
            virtual bool check_sym(const std::shared_ptr<const ContentsEntry> &) const;
            virtual bool check_misc(const std::shared_ptr<const ContentsEntry> &) const;

            ///\}

            ///\name Unlink operations
            ///\{

            virtual void unlink_file(FSPath, const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unlink_dir(FSPath, const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unlink_sym(FSPath, const std::shared_ptr<const ContentsEntry> &) const;
            virtual void unlink_misc(FSPath, const std::shared_ptr<const ContentsEntry> &) const;

            ///\}

            virtual void display(const std::string &) const = 0;

        public:
            ///\name Basic operations
            ///\{

            virtual ~Unmerger() = 0;

            ///\}

            /**
             * Perform the unmerge.
             */
            void unmerge();
    };
}

#endif
