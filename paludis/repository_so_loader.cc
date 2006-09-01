/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/about.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>

#include <dlfcn.h>
#include <list>
#include <algorithm>

using namespace paludis;

namespace
{
    class PaludisRepositorySoDirNotADirectoryError :
        public Exception
    {
        public:
            PaludisRepositorySoDirNotADirectoryError() throw () :
                Exception("PALUDIS_REPOSITORY_SO_DIR not a directory")
            {
            }
    };

    class PaludisRepositorySoDirCannotDlopenError :
        public Exception
    {
        private:
            std::string _file, _dlerr;
            mutable std::string _what;

        public:
            PaludisRepositorySoDirCannotDlopenError(const std::string & file,
                    const std::string & e) throw () :
                Exception("Cannot dlopen a repository. so file"),
                _file(file),
                _dlerr(e)
            {
            }

            ~PaludisRepositorySoDirCannotDlopenError() throw ()
            {
            }

            const char * what() const throw ()
            {
                if (_what.empty())
                    _what = std::string(Exception::what()) +
                        ": Cannot dlopen repository .so file '" + _file + "': '" + _dlerr + "'";
                return _what.c_str();
            }
    };

    struct RepositorySoLoader
    {
        RepositorySoLoader();
        ~RepositorySoLoader();

        void load_dir(const FSEntry &);

        std::list<void *> dl_opened;
    };

    static RepositorySoLoader repository_so_loader;
}

RepositorySoLoader::RepositorySoLoader()
{
    FSEntry so_dir(getenv_with_default("PALUDIS_REPOSITORY_SO_DIR",
                LIBDIR "/paludis/repositories"));

    if (! so_dir.is_directory())
        throw PaludisRepositorySoDirNotADirectoryError();

    load_dir(so_dir);
}

void
RepositorySoLoader::load_dir(const FSEntry & so_dir)
{
    for (DirIterator d(so_dir), d_end ; d != d_end ; ++d)
    {
        if (d->is_directory())
            load_dir(*d);

        if (! IsFileWithExtension(".so." + stringify(100 * PALUDIS_VERSION_MAJOR +
                        PALUDIS_VERSION_MINOR))(*d))
            continue;

        void * dl(dlopen(stringify(*d).c_str(), RTLD_LOCAL | RTLD_NOW));

        if (dl)
            dl_opened.push_back(dl);
        else
            throw PaludisRepositorySoDirCannotDlopenError(stringify(*d), dlerror());
    }

    if ((so_dir / ".libs").is_directory())
        load_dir(so_dir / ".libs");
}

RepositorySoLoader::~RepositorySoLoader()
{
    std::for_each(dl_opened.begin(), dl_opened.end(), &::dlclose);
}

