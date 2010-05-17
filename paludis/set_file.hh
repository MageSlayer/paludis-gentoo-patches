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

#ifndef PALUDIS_GUARD_PALUDIS_SET_FILE_HH
#define PALUDIS_GUARD_PALUDIS_SET_FILE_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tag-fwd.hh>
#include <tr1/functional>
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

    namespace n
    {
        struct environment;
        struct file_name;
        struct parser;
        struct set_operator_mode;
        struct tag;
        struct type;
    }

    /**
     * Parameters for a SetFile.
     *
     * \ingroup g_environment
     * \ingroup g_repository
     */
    struct SetFileParams
    {
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::file_name, FSEntry> file_name;
        NamedValue<n::parser, std::tr1::function<PackageDepSpec (const std::string &)> > parser;
        NamedValue<n::set_operator_mode, SetFileSetOperatorMode> set_operator_mode;
        NamedValue<n::tag, std::tr1::shared_ptr<const DepTag> > tag;
        NamedValue<n::type, SetFileType> type;
    };

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
            const std::tr1::shared_ptr<const SetSpecTree> contents() const;

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
             *
             * \since 0.48 returns whether any lines were removed
             */
            bool remove(const std::string &);
    };

    /**
     * Split a SetName into a SetName and a SetFileSetOperatorMode.
     *
     * \see SetName
     * \ingroup g_repository
     * \since 0.26
     */
    std::pair<SetName, SetFileSetOperatorMode> find_base_set_name_and_suffix_mode(const SetName &)
        PALUDIS_VISIBLE PALUDIS_ATTRIBUTE((warn_unused_result));
}

#endif
