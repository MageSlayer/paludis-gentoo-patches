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

#include <paludis/stripper.hh>
#include <paludis/stripper_extras.hh>
#include <paludis/about.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/log.hh>
#include <paludis/util/process.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/options.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/safe_ifstream.hh>
#include <functional>
#include <sstream>
#include <list>
#include <set>
#include <algorithm>
#include <sys/stat.h>

#include <dlfcn.h>
#include <stdint.h>

#include "config.h"

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;

typedef std::set<std::pair<dev_t, ino_t> > StrippedSet;

StripperError::StripperError(const std::string & s) noexcept :
    Exception(s)
{
}

namespace
{
    struct StripperHandle :
        Singleton<StripperHandle>
    {
        typedef PaludisStripperExtras * (* InitPtr) ();
        typedef const std::string (* LookupPtr) (PaludisStripperExtras * const, const std::string &);
        typedef void (* CleanupPtr) (PaludisStripperExtras * const);

        void * handle;
        InitPtr init;
        LookupPtr lookup;
        CleanupPtr cleanup;

        StripperHandle() :
            handle(nullptr),
            init(nullptr),
            lookup(nullptr),
            cleanup(nullptr)
        {
#ifndef ENABLE_STRIPPER
            Log::get_instance()->message("strip.unsupported", ll_warning, lc_context)
                << "Paludis was built without support for stripping. No stripping will be done.";
#else
            do
            {
                handle = ::dlopen(("libpaludisstripperextras_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
                if (! handle)
                    break;

                init = STUPID_CAST(InitPtr, ::dlsym(handle, "paludis_stripper_extras_init"));
                if (! init)
                    break;

                lookup = STUPID_CAST(LookupPtr, ::dlsym(handle, "paludis_stripper_extras_lookup"));
                if (! lookup)
                    break;

                cleanup = STUPID_CAST(CleanupPtr, ::dlsym(handle, "paludis_stripper_extras_cleanup"));
                if (! cleanup)
                    break;
            }
            while (false);

            if (! (handle && init && lookup && cleanup))
            {
                Log::get_instance()->message("strip.broken", ll_warning, lc_context)
                    << "Stripping cannot be used due to error '" << stringify(::dlerror()) << "' when using libpaludisstripperextras";

                if (handle)
                    ::dlclose(handle);
                handle = nullptr;
            }
#endif
        }

        ~StripperHandle()
        {
            if (handle)
                ::dlclose(handle);
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<Stripper>
    {
        bool potentially_need_strip;
        StripperOptions options;
        StrippedSet stripped_ids,
                    includes,
                    excludes;
        PaludisStripperExtras * stripper_extras;

        Imp(const StripperOptions & o) :
            options(o),
            stripper_extras(nullptr)
        {
            /* If stripping is disabled, there's nothing else to do. */
            potentially_need_strip = o.strip_choice();

            /*
             * Otherwise, the ebuild (if it is one) might be strip-restricted,
             * but we don't care about this if controllable strip is turned
             * on.
             */
            potentially_need_strip &= o.controllable_strip()
                                    || (! o.strip_restrict());

            try
            {
                if (StripperHandle::get_instance()->handle)
                    stripper_extras = StripperHandle::get_instance()->init();
            }
            catch (const StripperError & e)
            {
                Log::get_instance()->message("strip.broken", ll_warning, lc_context)
                    << "Got error '" << e.message() << "' (" << e.what() << ") when attempting to strip";
                stripper_extras = nullptr;
            }
        }

        ~Imp()
        {
            if (stripper_extras)
                StripperHandle::get_instance()->cleanup(stripper_extras);
        }
    };
}

Stripper::Stripper(const StripperOptions & options) :
    _imp(options)
{
}

Stripper::~Stripper() = default;

void
Stripper::strip()
{
    Context context("When stripping image '" + stringify(_imp->options.image_dir()) + "':");

    if (! _imp->potentially_need_strip)
        return;

    /* Build the two lists. */
    populate_list(true);
    populate_list(false);

    do_dir_recursive(_imp->options.image_dir(), false);
}

void
Stripper::do_dir_recursive(const FSPath & f, const bool parent_included)
{
    Context context("When stripping inside '" + stringify(f) + "':");

    if (f == _imp->options.debug_dir())
        return;

    on_enter_dir(f);

    /*
     * The current path is either:
     *   - excluded if part of the excludes list
     *   - automatically included if the parent is included
     *   - included if part of the includes list
     *   - otherwise excluded.
     */
    bool path_included = (!(part_of_list(f, false)))
                       && (parent_included || part_of_list(f, true));

    for (FSIterator d(f, { fsio_include_dotfiles, fsio_inode_sort }), d_end ; d != d_end ; ++d)
    {
        FSStat d_stat(*d);

        if (d_stat.is_symlink())
            continue;
        if (_imp->stripped_ids.end() != _imp->stripped_ids.find(d_stat.lowlevel_id()))
            continue;

        /* Same logic as above. */
        path_included = (!(part_of_list(*d, false)))
                      && (path_included || part_of_list(*d, true));

        if (d_stat.is_directory())
        {
            do_dir_recursive(*d, path_included);
        }
        else if (d_stat.is_regular_file() && path_included)
        {
            if ((0 != (d_stat.permissions() & (S_IXUSR | S_IXGRP | S_IXOTH))) ||
                    (std::string::npos != d->basename().find(".so.")) ||
                    (d->basename() != strip_trailing_string(d->basename(), ".so")))
            {
                std::string t(file_type(*d));
                if (std::string::npos != t.find("SB executable") || std::string::npos != t.find("SB shared object") ||
                                std::string::npos != t.find("SB pie executable"))
                {
                    if (_imp->options.dwarf_compression())
                        do_dwarf_compress(*d);
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

    if (_imp->stripper_extras)
    {
        std::string result(StripperHandle::get_instance()->lookup(_imp->stripper_extras, stringify(f)));
        Log::get_instance()->message("strip.type", ll_debug, lc_context)
            << "Magic says '" << f << "' is '" << result << "'";
        return result;
    }
    else
        return "";
}

void
Stripper::do_strip(const FSPath & f, const std::string & options)
{
    const std::string strip_tool(_imp->options.tool_prefix() + "strip");
    Context context("When stripping '" + stringify(f) + "':");
    on_strip(f);

    Process strip_process(options.empty()
                            ? ProcessCommand({ strip_tool, stringify(f) })
                            : ProcessCommand({ strip_tool, options, stringify(f) }));
    if (0 != strip_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't strip '" << f << "'";
    _imp->stripped_ids.insert(f.stat().lowlevel_id());
}

void
Stripper::do_split(const FSPath & f, const FSPath & g)
{
    const std::string objcopy_tool(_imp->options.tool_prefix() + "objcopy");
    Context context("When splitting '" + stringify(f) + "' to '" + stringify(g) + "':");
    on_split(f, g);

    {
        std::list<FSPath> to_make;
        for (FSPath d(g.dirname()) ; (! d.stat().exists()) && (d != _imp->options.image_dir()) ; d = d.dirname())
            to_make.push_front(d);

        using namespace std::placeholders;
        std::for_each(to_make.begin(), to_make.end(), std::bind(std::mem_fn(&FSPath::mkdir), _1, 0755, FSPathMkdirOptions() + fspmkdo_ok_if_exists));
    }

    ProcessCommand objcopy_copy_process_args({ objcopy_tool, "--only-keep-debug", stringify(f), stringify(g) });
    if (_imp->options.compress_splits())
        objcopy_copy_process_args.append_args({ "--compress-debug-sections" });
    Process objcopy_copy_process(std::move(objcopy_copy_process_args));

    Process objcopy_link_process(ProcessCommand({ objcopy_tool, "--add-gnu-debuglink=" + stringify(g), stringify(f) }));

    if (0 != objcopy_copy_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't copy debug information for '" << f << "'";
    else if (0 != objcopy_link_process.run().wait())
        Log::get_instance()->message("strip.failure", ll_warning, lc_context) << "Couldn't add debug link for '" << f << "'";
    else
        g.chmod(g.stat().permissions() & ~(S_IXGRP | S_IXUSR | S_IXOTH | S_IWOTH));
}

void
Stripper::do_dwarf_compress(const FSPath & f)
{
    Context context("When compressing DWARF information for '" + stringify(f) + "'");

    on_dwarf_compress(f);

    Process dwz_process(ProcessCommand({ "dwz", /* quiet => */ "-q", stringify(f) }));
    if (dwz_process.run().wait() != 0)
        Log::get_instance()->message("strip.failure", ll_warning, lc_context)
            << "Couldn't compress DWARF information for '" << f << "'";
}

std::string
Stripper::strip_action_desc() const
{
    return "str";
}

std::string
Stripper::split_action_desc() const
{
    const char desc[3] = {
        _imp->options.dwarf_compression() ? 'd' : 's',
        'p',
        _imp->options.compress_splits() ? 'z' : 'l',
    };
    return std::string(desc, sizeof(desc));
}

std::string
Stripper::unknown_action_desc() const
{
    return "---";
}

std::string
Stripper::dwarf_compress_desc() const
{
    return "dwz";
}

void
Stripper::populate_list(const bool include)
{
    Context context(std::string("When generating ") + (include ? std::string("include") : std::string("exclude")) + std::string(" list for stripper:"));

    std::string file_name = "dostrip.";

    auto list = &_imp->includes;
    if (include)
    {
        file_name += "include";

        /*
         * Add the whole image dir to the include list if the package is not
         * strip restricted.
         */
        if (!(_imp->options.strip_restrict()))
        {
            list->insert(_imp->options.image_dir().stat().lowlevel_id());
        }
    }
    else
    {
        file_name += "exclude";

        list = &_imp->excludes;
    }

    /* If controllable strip is not a thing, move out early. */
    if (!(_imp->options.controllable_strip()))
    {
        return;
    }

    /* Otherwise, chug on and actually read and process the files. */
    FSPath file(_imp->options.controllable_strip_dir());
    file /= file_name;

    if (!(file.stat().exists()))
    {
        /*
         * File does not exist, which is fine, and probably means that dostrip
         * was never called. Handle this gracefully.
         */
        return;
    }

    try
    {
        SafeIFStream file_stream(file);
        std::string elem;
        while (std::getline(file_stream, elem, '\0'))
        {
            FSStat stat(FSPath(elem).stat());

            if (stat.exists())
            {
                list->insert(stat.lowlevel_id());
            }
        }
    }
    catch (const SafeIFStreamError & e)
    {
        Log::get_instance()->message("stripper.strip.populate_list", ll_warning, lc_context) << e.message();
        throw InternalError(PALUDIS_HERE, "Unable to read from " + file_name);
    }
}

bool
Stripper::part_of_list(const FSPath &entry, const bool includes)
{
    bool ret = false;

    auto list = &_imp->includes;

    if (!(includes)) {
        list = &_imp->excludes;
    }

    auto lowlevel_id = entry.stat().lowlevel_id();

    /* Check if the current entry is part of the selected list. */
    ret = (list->end() != list->find(lowlevel_id));

    return ret;
}
