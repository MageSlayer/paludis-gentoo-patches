/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_SET_FILE_HH
#define PALUDIS_GUARD_PALUDIS_SET_FILE_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/name.hh>
#include <paludis/dep_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tag-fwd.hh>
#include <iosfwd>

/** \file
 * Declarations for the SetFile classes, which are used by Environment and
 * Repository implementations for files containing a package set.
 *
 * \ingroup g_environment
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;

#include <paludis/set_file-se.hh>
#include <paludis/set_file-sr.hh>

    /**
     * Thrown if there is a problem reading or writing a SetFile.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SetFileError :
        public ConfigurationError
    {
        public:
            ///\name Basic operations
            ///\{

            SetFileError(const FSEntry &, const std::string &) throw ();

            ///\}
    };

    /**
     * Shared code for files containing a package set.
     *
     * Various set file formats are supported:
     *
     * - sft_paludis_conf, a line-based set file with prefixed entries
     * - sft_paludis_bash, a bash script that outputs an sft_paludis_conf
     * - sft_simple, a simple line-based file
     *
     * The file can be modified if it is sft_paludis_conf or sft_simple.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE SetFile :
        private InstantiationPolicy<SetFile, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<SetFile>
    {
        public:
            ///\name Basic operations
            ///\{

            SetFile(const SetFileParams &);
            ~SetFile();

            ///\}

            /**
             * Fetch our contents.
             */
            tr1::shared_ptr<SetSpecTree::ConstItem> contents() const;

            /**
             * Rewrite our contents.
             */
            void rewrite() const;

            /**
             * Add an item to our contents, if it is not there already.
             */
            void add(const std::string &);

            /**
             * Remove any matching lines.
             */
            void remove(const std::string &);
    };
}

#endif
