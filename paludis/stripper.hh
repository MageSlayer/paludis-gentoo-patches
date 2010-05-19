/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct debug_dir_name> debug_dir;
        typedef Name<struct image_dir_name> image_dir;
        typedef Name<struct split_name> split;
        typedef Name<struct strip_name> strip;
    }

    struct StripperOptions
    {
        NamedValue<n::debug_dir, FSEntry> debug_dir;
        NamedValue<n::image_dir, FSEntry> image_dir;
        NamedValue<n::split, bool> split;
        NamedValue<n::strip, bool> strip;
    };

    class PALUDIS_VISIBLE Stripper :
        private PrivateImplementationPattern<Stripper>
    {
        protected:
            virtual void on_enter_dir(const FSEntry &) = 0;
            virtual void on_leave_dir(const FSEntry &) = 0;

            virtual void on_strip(const FSEntry &) = 0;
            virtual void on_split(const FSEntry &, const FSEntry &) = 0;
            virtual void on_unknown(const FSEntry &) = 0;

            virtual void do_dir_recursive(const FSEntry &);

            virtual std::string file_type(const FSEntry &);

            virtual void do_split(const FSEntry &, const FSEntry &);
            virtual void do_strip(const FSEntry &, const std::string &);

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
