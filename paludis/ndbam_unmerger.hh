/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_UNMERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_UNMERGER_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/unmerger.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <functional>

namespace paludis
{
    namespace n
    {
        typedef Name<struct config_protect_name> config_protect;
        typedef Name<struct config_protect_mask_name> config_protect_mask;
        typedef Name<struct contents_file_name> contents_file;
        typedef Name<struct environment_name> environment;
        typedef Name<struct ignore_name> ignore;
        typedef Name<struct ndbam_name> ndbam;
        typedef Name<struct output_manager_name> output_manager;
        typedef Name<struct package_id_name> package_id;
        typedef Name<struct root_name> root;
    }

    class NDBAM;

    struct NDBAMUnmergerOptions
    {
        NamedValue<n::config_protect, std::string> config_protect;
        NamedValue<n::config_protect_mask, std::string> config_protect_mask;
        NamedValue<n::contents_file, FSEntry> contents_file;
        NamedValue<n::environment, const Environment *> environment;
        NamedValue<n::ignore, const std::function<bool (const FSEntry &)> > ignore;
        NamedValue<n::ndbam, const NDBAM *> ndbam;
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
        NamedValue<n::root, FSEntry> root;
    };

    class PALUDIS_VISIBLE NDBAMUnmergerError :
        public UnmergerError
    {
        public:
            NDBAMUnmergerError(const std::string &) throw ();
    };

    /**
     * Unmerger implementation for NDBAM.
     *
     * \ingroup g_ndbam
     * \since 0.26
     */
    class PALUDIS_VISIBLE NDBAMUnmerger :
        public Unmerger,
        private PrivateImplementationPattern<NDBAMUnmerger>
    {
        private:
            Implementation<NDBAMUnmerger> * _imp;

            void _add_file(const std::shared_ptr<const ContentsEntry> &);
            void _add_dir(const std::shared_ptr<const ContentsEntry> &);
            void _add_sym(const std::shared_ptr<const ContentsEntry> &);

        protected:
            bool config_protected(const FSEntry &) const;
            std::string make_tidy(const FSEntry &) const;

            void populate_unmerge_set();

            void display(const std::string &) const;

            bool check_file(const std::shared_ptr<const ContentsEntry> &) const;
            bool check_dir(const std::shared_ptr<const ContentsEntry> &) const;
            bool check_sym(const std::shared_ptr<const ContentsEntry> &) const;
            bool check_misc(const std::shared_ptr<const ContentsEntry> &) const;

        public:
            ///\name Basic operations
            ///\{

            NDBAMUnmerger(const NDBAMUnmergerOptions &);
            ~NDBAMUnmerger();

            ///\}

            virtual Hook extend_hook(const Hook &) const;
    };
}

#endif
