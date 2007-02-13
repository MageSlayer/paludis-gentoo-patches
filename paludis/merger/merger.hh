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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_MERGER_HH 1

#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/exception.hh>
#include <iosfwd>

namespace paludis
{
    class Environment;
    class Hook;

#include <paludis/merger/merger-se.hh>
#include <paludis/merger/merger-sr.hh>

    class MergerError :
        public Exception
    {
        public:
            MergerError(const std::string & msg) throw ();
    };

    class Merger
    {
        private:
            MergerOptions _options;
            bool _result;

        protected:
            Merger(const MergerOptions &);

            void make_check_fail();

            virtual Hook extend_hook(const Hook &);

            virtual MergerEntryType entry_type(const FSEntry &);

            virtual void do_dir_recursive(bool is_check, const FSEntry &, const FSEntry &);

            virtual void on_enter_dir(bool is_check, const FSEntry);
            virtual void on_leave_dir(bool is_check, const FSEntry);

            virtual void on_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual void install_file(const FSEntry &, const FSEntry &, const std::string &);
            virtual void unlink_file(FSEntry);
            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &) = 0;

            virtual void on_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual void install_dir(const FSEntry &, const FSEntry &);
            virtual void unlink_dir(FSEntry);
            virtual void record_install_dir(const FSEntry &, const FSEntry &) = 0;

            virtual void on_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual void install_sym(const FSEntry &, const FSEntry &);
            virtual void unlink_sym(FSEntry);
            virtual void record_install_sym(const FSEntry &, const FSEntry &) = 0;

            virtual void unlink_misc(FSEntry);
            virtual void on_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual void on_error(bool is_check, const std::string &) = 0;
            virtual void on_warn(bool is_check, const std::string &) = 0;

            virtual bool config_protected(const FSEntry &, const FSEntry &) = 0;
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &) = 0;

        public:
            virtual ~Merger();

            virtual bool check() PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void merge();
    };

}

#endif
