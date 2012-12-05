/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_STRIPPER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_STRIPPER_HH 1

#include <paludis/stripper.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_compress_splits> compress_splits;
        typedef Name<struct name_debug_dir> debug_dir;
        typedef Name<struct name_dwarf_compression> dwarf_compression;
        typedef Name<struct name_image_dir> image_dir;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_split> split;
        typedef Name<struct name_strip> strip;
    }

    namespace erepository
    {
        struct EStripperOptions
        {
            NamedValue<n::compress_splits, bool> compress_splits;
            NamedValue<n::debug_dir, FSPath> debug_dir;
            NamedValue<n::dwarf_compression, bool> dwarf_compression;
            NamedValue<n::image_dir, FSPath> image_dir;
            NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
            NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
            NamedValue<n::split, bool> split;
            NamedValue<n::strip, bool> strip;
        };

        class EStripper :
            public Stripper
        {
            private:
                Pimp<EStripper> _imp;

            protected:
                virtual void on_enter_dir(const FSPath &);
                virtual void on_leave_dir(const FSPath &);

                virtual void on_strip(const FSPath &);
                virtual void on_split(const FSPath &, const FSPath &);
                virtual void on_dwarf_compress(const FSPath &);
                virtual void on_unknown(const FSPath &);

            public:
                EStripper(const EStripperOptions &);
                ~EStripper();

                virtual void strip();
        };
    }
}

#endif
