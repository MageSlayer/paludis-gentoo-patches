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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_UNMERGER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sr.hh>

namespace paludis
{
    class Hook;
    class Environment;

#include <paludis/merger/unmerger-sr.hh>

    /**
     * Thrown if an error occurs during an unmerge.
     *
     * \ingroup grpunmerger
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class UnmergerError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            UnmergerError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Handles unmerging items.
     *
     * \ingroup grpunmerger
     * \nosubgrouping
     */
    class Unmerger
    {
        private:
            UnmergerOptions _options;

        protected:
            ///\name Basic operations
            ///\{

            Unmerger(const UnmergerOptions &);

            ///\}

            /**
             * Extend a hook with extra options.
             */
            virtual Hook extend_hook(const Hook &);

            ///\name Unlink operations
            ///\{

            virtual void unlink_file(FSEntry);
            virtual void unlink_dir(FSEntry);
            virtual void unlink_sym(FSEntry);
            virtual void unlink_misc(FSEntry);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Unmerger();

            ///\}
    };
}

#endif
