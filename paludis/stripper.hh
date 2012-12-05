/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_STRIPPER_HH
#define PALUDIS_GUARD_PALUDIS_STRIPPER_HH 1

#include <paludis/stripper-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/exception.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_compress_splits> compress_splits;
        typedef Name<struct name_debug_dir> debug_dir;
        typedef Name<struct name_dwarf_compression> dwarf_compression;
        typedef Name<struct name_image_dir> image_dir;
        typedef Name<struct name_split> split;
        typedef Name<struct name_strip> strip;
    }

    struct StripperOptions
    {
        NamedValue<n::compress_splits, bool> compress_splits;
        NamedValue<n::debug_dir, FSPath> debug_dir;
        NamedValue<n::dwarf_compression, bool> dwarf_compression;
        NamedValue<n::image_dir, FSPath> image_dir;
        NamedValue<n::split, bool> split;
        NamedValue<n::strip, bool> strip;
    };

    class PALUDIS_VISIBLE StripperError :
        public Exception
    {
        public:
            StripperError(const std::string &) throw ();
    };

    class PALUDIS_VISIBLE Stripper
    {
        private:
            Pimp<Stripper> _imp;

        protected:
            virtual void on_enter_dir(const FSPath &) = 0;
            virtual void on_leave_dir(const FSPath &) = 0;

            virtual void on_strip(const FSPath &) = 0;
            virtual void on_split(const FSPath &, const FSPath &) = 0;
            virtual void on_dwarf_compress(const FSPath &) = 0;
            virtual void on_unknown(const FSPath &) = 0;

            virtual void do_dir_recursive(const FSPath &);

            virtual std::string file_type(const FSPath &);

            virtual void do_split(const FSPath &, const FSPath &);
            virtual void do_strip(const FSPath &, const std::string &);
            virtual void do_dwarf_compress(const FSPath &);

            virtual std::string strip_action_desc() const;
            virtual std::string split_action_desc() const;
            virtual std::string unknown_action_desc() const;
            virtual std::string dwarf_compress_desc() const;

        public:
            ///\name Basic operations
            ///\{

            Stripper(const StripperOptions &);
            virtual ~Stripper() = 0;

            ///\}

            /**
             * Perform the strip.
             */
            virtual void strip();
    };
}

#endif
