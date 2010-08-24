/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/tar_merger.hh>
#include <paludis/tar_extras.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/stringify.hh>
#include <paludis/about.hh>

#include <ostream>

#include <dlfcn.h>
#include <stdint.h>

#include "config.h"

#define STUPID_CAST(type, val) reinterpret_cast<type>(reinterpret_cast<uintptr_t>(val))

using namespace paludis;

#include <paludis/tar_merger-se.cc>

namespace
{
    struct TarMergerHandle :
        Singleton<TarMergerHandle>
    {
        typedef PaludisTarExtras * (* InitPtr) (const std::string &, const std::string &);
        typedef void (* AddFilePtr) (PaludisTarExtras * const, const std::string &, const std::string &);
        typedef void (* AddSymPtr) (PaludisTarExtras * const, const std::string &, const std::string &, const std::string &);
        typedef void (* CleanupPtr) (PaludisTarExtras * const);

        void * handle;
        InitPtr init;
        AddFilePtr add_file;
        AddSymPtr add_sym;
        CleanupPtr cleanup;

        TarMergerHandle()
        {
#ifndef ENABLE_PBINS
            throw NotAvailableError("Paludis was built without support for pbins");
#endif

            handle = ::dlopen(("libpaludistarextras_" + stringify(PALUDIS_PC_SLOT) + ".so").c_str(), RTLD_NOW | RTLD_GLOBAL);
            if (! handle)
                throw MergerError("Unable to open libpaludistarextras due to error '" + stringify(::dlerror()) + "' from dlopen");

            init = STUPID_CAST(InitPtr, ::dlsym(handle, "paludis_tar_extras_init"));
            if (! init)
                throw MergerError("Unable to init from libpaludistarextras due to error '" + stringify(::dlerror()) + "' from dlsym");

            add_file = STUPID_CAST(AddFilePtr, ::dlsym(handle, "paludis_tar_extras_add_file"));
            if (! add_file)
                throw MergerError("Unable to add_file from libpaludistarextras due to error '" + stringify(::dlerror()) + "' from dlsym");

            add_sym = STUPID_CAST(AddSymPtr, ::dlsym(handle, "paludis_tar_extras_add_sym"));
            if (! add_sym)
                throw MergerError("Unable to add_sym from libpaludistarextras due to error '" + stringify(::dlerror()) + "' from dlsym");

            cleanup = STUPID_CAST(CleanupPtr, ::dlsym(handle, "paludis_tar_extras_cleanup"));
            if (! cleanup)
                throw MergerError("Unable to cleanup from libpaludistarextras due to error '" + stringify(::dlerror()) + "' from dlsym");
        }

        ~TarMergerHandle()
        {
            if (handle)
                ::dlclose(handle);
        }
    };
}

namespace paludis
{
    template <>
    struct Imp<TarMerger>
    {
        TarMergerParams params;
        PaludisTarExtras * tar;

        Imp(const TarMergerParams & p) :
            params(p)
        {
        }
    };
}

TarMerger::TarMerger(const TarMergerParams & p) :
    Pimp<TarMerger>(p),
    Merger(make_named_values<MergerParams>(
                n::environment() = p.environment(),
                n::fix_mtimes_before() = p.fix_mtimes_before(),
                n::get_new_ids_or_minus_one() = p.get_new_ids_or_minus_one(),
                n::image() = p.image(),
                n::install_under() = FSPath("/"),
                n::maybe_output_manager() = p.maybe_output_manager(),
                n::merged_entries() = p.merged_entries(),
                n::no_chown() = p.no_chown(),
                n::options() = p.options(),
                n::root() = p.root()
                )),
    _imp(Pimp<TarMerger>::_imp)
{
}

TarMerger::~TarMerger() = default;

void
TarMerger::on_file_main(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    add_file(src, dst / src.basename());
}

void
TarMerger::add_file(const FSPath & src, const FSPath & dst)
{
    track_install_file(src, dst);
    (*TarMergerHandle::get_instance()->add_file)(_imp->tar, stringify(src), strip_leading(stringify(dst), "/"));
}

void
TarMerger::on_dir_main(bool, const FSPath &, const FSPath &)
{
}

void
TarMerger::on_sym_main(bool is_check, const FSPath & src, const FSPath & dst)
{
    if (is_check)
        return;

    track_install_sym(src, dst / src.basename());
    (*TarMergerHandle::get_instance()->add_sym)(_imp->tar, stringify(src), strip_leading(stringify(dst / src.basename()), "/"),
            stringify(src.readlink()));
}

void
TarMerger::prepare_install_under()
{
}

void
TarMerger::merge()
{
    std::string compress;

    switch (_imp->params.compression())
    {
        case tmc_none:
            compress = "none";
            break;

        case tmc_bz2:
            compress = "bz2";
            break;

        case last_tmc:
            break;
    };

    if (compress.empty())
        throw InternalError(PALUDIS_HERE, "unknown compress");

    _imp->tar = (*TarMergerHandle::get_instance()->init)(stringify(_imp->params.tar_file()), compress);

    try
    {
        Merger::merge();

        (*TarMergerHandle::get_instance()->cleanup)(_imp->tar);
    }
    catch (...)
    {
        (*TarMergerHandle::get_instance()->cleanup)(_imp->tar);
        throw;
    }
}

FSPath
TarMerger::canonicalise_root_path(const FSPath & f)
{
    return f;
}

