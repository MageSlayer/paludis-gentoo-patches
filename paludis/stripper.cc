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

#include <paludis/stripper.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/process.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/options.hh>
#include <functional>
#include <sstream>
#include <list>
#include <set>
#include <algorithm>
#include <sys/stat.h>

using namespace paludis;

typedef std::set<std::pair<dev_t, ino_t> > StrippedSet;

namespace paludis
{
    template <>
    struct Imp<Stripper>
    {
        StripperOptions options;
        StrippedSet stripped_ids;

        Imp(const StripperOptions & o) :
            options(o)
        {
        }
    };
}

Stripper::Stripper(const StripperOptions & options) :
    Pimp<Stripper>(options)
{
}

Stripper::~Stripper()
{
}

void
Stripper::strip()
{
    Context context("When stripping image '" + stringify(_imp->options.image_dir()) + "':");

    if (! _imp->options.strip())
        return;

    do_dir_recursive(_imp->options.image_dir());
}

void
Stripper::do_dir_recursive(const FSPath & f)
{
    Context context("When stripping inside '" + stringify(f) + "':");

    if (f == _imp->options.debug_dir())
        return;

    on_enter_dir(f);

    for (FSIterator d(f, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
    {
        FSStat d_stat(*d);

        if (d_stat.is_symlink())
            continue;
        if (_imp->stripped_ids.end() != _imp->stripped_ids.find(d_stat.lowlevel_id()))
            continue;

        if (d_stat.is_directory())
            do_dir_recursive(*d);

        else if (d_stat.is_regular_file())
        {
            if ((0 != (d_stat.permissions() & (S_IXUSR | S_IXGRP | S_IXOTH))) ||
                    (std::string::npos != d->basename().find(".so.")) ||
                    (d->basename() != strip_trailing_string(d->basename(), ".so")))
            {
                std::string t(file_type(*d));
                if (std::string::npos != t.find("SB executable") || std::string::npos != t.find("SB shared object"))
                {
                    if (_imp->options.split())
                    {
                        FSPath target(_imp->options.debug_dir() / d->strip_leading(_imp->options.image_dir()));
                        target = target.dirname() / (target.basename() + ".debug");
                        do_split(*d, target);
                    }
                    do_strip(*d, "");
                }
                else if (std::string::npos != t.find("current ar archive"))
                {
                    do_strip(*d, "-g");
                }
                else
                    on_unknown(*d);
            }
        }
    }

    on_leave_dir(f);
}

std::string
Stripper::file_type(const FSPath & f)
{
    Context context("When finding the file type of '" + stringify(f) + "':");

    std::stringstream s;

    Process file_process(ProcessCommand({ "file", stringify(f) }));
    file_process.capture_stdout(s);
    if (0 != file_process.run().wait())
        Log::get_instance()->message("strip.identification_failed", ll_warning, lc_context)
            << "Couldn't work out the file type of '" << f << "'";

    return s.str();
}

void
Stripper::do_strip(const FSPath & f, const std::string & options)
{
    Context context("When stripping '" + stringify(f) + "':");
    on_strip(f);

    Process strip_process(options.empty() ?
            ProcessCommand({ "strip", stringify(f) }) :
            ProcessCommand({ "strip", options, stringify(f) }));
    if (0 != strip_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't strip '" << f << "'";
    _imp->stripped_ids.insert(f.stat().lowlevel_id());
}

void
Stripper::do_split(const FSPath & f, const FSPath & g)
{
    Context context("When splitting '" + stringify(f) + "' to '" + stringify(g) + "':");
    on_split(f, g);

    {
        std::list<FSPath> to_make;
        for (FSPath d(g.dirname()) ; (! d.stat().exists()) && (d != _imp->options.image_dir()) ; d = d.dirname())
            to_make.push_front(d);

        using namespace std::placeholders;
        std::for_each(to_make.begin(), to_make.end(), std::bind(std::mem_fn(&FSPath::mkdir), _1, 0755, FSPathMkdirOptions() + fspmkdo_ok_if_exists));
    }

    Process objcopy_copy_process(ProcessCommand({ "objcopy", "--only-keep-debug", stringify(f), stringify(g) }));
    Process objcopy_link_process(ProcessCommand({ "objcopy", "--add-gnu-debuglink=" + stringify(g), stringify(f) }));
    if (0 != objcopy_copy_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't copy debug information for '" << f << "'";
    else if (0 != objcopy_link_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't add debug link for '" << f << "'";
    else
        g.chmod(g.stat().permissions() & ~(S_IXGRP | S_IXUSR | S_IXOTH | S_IWOTH));
}

