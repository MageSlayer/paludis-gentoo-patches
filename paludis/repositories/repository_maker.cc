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

#include "repository_maker.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/about.hh>
#include <list>
#include <dlfcn.h>
#include <stdint.h>

#include "config.h"

#ifdef MONOLITHIC
#  include <paludis/repositories/portage/make_ebuild_repository.hh>
#  include <paludis/repositories/vdb/vdb_repository.hh>
#  include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#  include <paludis/repositories/virtuals/virtuals_repository.hh>
#endif

using namespace paludis;

PaludisRepositorySoDirNotADirectoryError::PaludisRepositorySoDirNotADirectoryError() throw () :
    Exception("PALUDIS_REPOSITORY_SO_DIR not a directory")
{
}

PaludisRepositorySoDirCannotDlopenError::PaludisRepositorySoDirCannotDlopenError(
        const std::string & file, const std::string & e) throw () :
    Exception("Cannot dlopen a repository. so file"),
    _file(file),
    _dlerr(e)
{
}

PaludisRepositorySoDirCannotDlopenError::~PaludisRepositorySoDirCannotDlopenError() throw ()
{
}

const char *
PaludisRepositorySoDirCannotDlopenError::what() const throw ()
{
    if (_what.empty())
        _what = std::string(Exception::what()) +
            ": Cannot dlopen repository .so file '" + _file + "': '" + _dlerr + "'";
    return _what.c_str();
}

namespace paludis
{
    template<>
    struct Implementation<RepositoryMaker> :
        InternalCounted<Implementation<RepositoryMaker> >
    {
        std::list<void *> dl_opened;
    };
}

void
RepositoryMaker::load_dir(const FSEntry & so_dir)
{
    for (DirIterator d(so_dir), d_end ; d != d_end ; ++d)
    {
        if (d->is_directory())
            load_dir(*d);

        if (! IsFileWithExtension(".so." + stringify(100 * PALUDIS_VERSION_MAJOR +
                        PALUDIS_VERSION_MINOR))(*d))
            continue;

        /* don't use RTLD_LOCAL, g++ is over happy about template instantiations, and it
         * can lead to multiple singleton instances. */
        void * dl(dlopen(stringify(*d).c_str(), RTLD_GLOBAL | RTLD_NOW));

        if (dl)
        {
            _imp->dl_opened.push_back(dl);

            void * reg(dlsym(dl, "register_repositories"));
            if (reg)
            {
                reinterpret_cast<void (*)(RepositoryMaker *)>(
                        reinterpret_cast<uintptr_t>(reg))(this);
            }
            else
                throw PaludisRepositorySoDirCannotDlopenError(stringify(*d),
                        "no register_repositories function defined");
        }
        else
            throw PaludisRepositorySoDirCannotDlopenError(stringify(*d), dlerror());
    }

    if ((so_dir / ".libs").is_directory())
        load_dir(so_dir / ".libs");
}

RepositoryMaker::RepositoryMaker() :
    PrivateImplementationPattern<RepositoryMaker>(new Implementation<RepositoryMaker>)
{
#ifdef MONOLITHIC

    register_maker("ebuild", &make_ebuild_repository_wrapped);
    register_maker("vdb", &VDBRepository::make_vdb_repository);
    register_maker("virtuals", &VirtualsRepository::make_virtuals_repository);
    register_maker("installed_virtuals", &InstalledVirtualsRepository::make_installed_virtuals_repository);

#else
    FSEntry so_dir(getenv_with_default("PALUDIS_REPOSITORY_SO_DIR", LIBDIR "/paludis/repositories"));

    if (! so_dir.is_directory())
        throw PaludisRepositorySoDirNotADirectoryError();

    load_dir(so_dir);

#endif
}

RepositoryMaker::~RepositoryMaker()
{
}

#ifndef MONOLITHIC

extern "C"
{
    void register_repositories(RepositoryMaker * maker);
}

void register_repositories(RepositoryMaker *)
{
}

#endif

