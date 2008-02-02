/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_MERGER_HH 1

#include <paludis/merger.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
#include <paludis/ndbam_merger-sr.hh>

    class PALUDIS_VISIBLE NDBAMMerger :
        public Merger,
        private PrivateImplementationPattern<NDBAMMerger>
    {
        private:
            void display_override(const std::string &) const;
            std::string make_arrows(const MergeStatusFlags &) const;

        public:
            NDBAMMerger(const NDBAMMergerOptions &);
            ~NDBAMMerger();

            virtual Hook extend_hook(const Hook &);

            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const MergeStatusFlags &);
            virtual void record_install_dir(const FSEntry &, const FSEntry &, const MergeStatusFlags &);
            virtual void record_install_sym(const FSEntry &, const FSEntry &, const MergeStatusFlags &);

            virtual void on_error(bool is_check, const std::string &);
            virtual void on_warn(bool is_check, const std::string &);
            virtual void on_enter_dir(bool is_check, const FSEntry);

            virtual bool config_protected(const FSEntry &, const FSEntry &);
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &);

            virtual void merge();
            virtual bool check();
    };
}

#endif
