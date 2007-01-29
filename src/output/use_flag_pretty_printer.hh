/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_USE_FLAG_PRETTY_PRINTER_HH
#define PALUDIS_GUARD_SRC_USE_FLAG_PRETTY_PRINTER_HH 1

#include <paludis/name.hh>
#include <iosfwd>

namespace paludis
{
    class Environment;
    class PackageDatabaseEntry;

    class UseFlagPrettyPrinter
    {
        private:
            const Environment * const _env;
            bool _need_space;

            std::tr1::shared_ptr<UseFlagNameCollection> _new_flags;
            std::tr1::shared_ptr<UseFlagNameCollection> _changed_flags;
            std::tr1::shared_ptr<UseFlagNameCollection> _unchanged_flags;
            std::tr1::shared_ptr<UseFlagNameCollection> _expand_prefixes;

        protected:
            std::string::size_type use_expand_delim_pos(const UseFlagName & u,
                    const std::tr1::shared_ptr<const UseFlagNameCollection> c) const;

        public:
            UseFlagPrettyPrinter(const Environment * const);
            virtual ~UseFlagPrettyPrinter();

            virtual void print_package_flags(const PackageDatabaseEntry &,
                    const PackageDatabaseEntry * const = 0);

            virtual void output_flag(const std::string &);
            virtual void output_flag_changed_mark();
            virtual void output_flag_is_new_mark();
            virtual void output_expand_prefix(const std::string &);

            virtual std::string render_as_enabled_flag(const std::string &) const;
            virtual std::string render_as_disabled_flag(const std::string &) const;
            virtual std::string render_as_forced_flag(const std::string &) const;
            virtual std::string render_as_masked_flag(const std::string &) const;

            virtual std::ostream & output_stream() const;

            const Environment * environment() const;
            bool need_space() const;

            std::tr1::shared_ptr<const UseFlagNameCollection> new_flags() const;
            std::tr1::shared_ptr<const UseFlagNameCollection> changed_flags() const;
            std::tr1::shared_ptr<const UseFlagNameCollection> unchanged_flags() const;
            std::tr1::shared_ptr<const UseFlagNameCollection> expand_prefixes() const;
    };
}

#endif
