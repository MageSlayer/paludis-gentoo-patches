/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "environment_maker.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/distribution.hh>
#include <paludis/about.hh>
#include <list>
#include <set>
#include <dlfcn.h>
#include <stdint.h>

using namespace paludis;

template class VirtualConstructor<std::string,
         tr1::shared_ptr<Environment> (*) (const std::string &),
         virtual_constructor_not_found::ThrowException<NoSuchEnvironmentTypeError> >;

NoSuchEnvironmentTypeError::NoSuchEnvironmentTypeError(const std::string & format) throw ():
    ConfigurationError("No available maker for environment type '" + format + "'")
{
}

PaludisEnvironmentSoDirNotADirectoryError::PaludisEnvironmentSoDirNotADirectoryError() throw () :
    Exception("PALUDIS_ENVIRONMENT_SO_DIR not a directory")
{
}

PaludisEnvironmentSoDirCannotDlopenError::PaludisEnvironmentSoDirCannotDlopenError(
        const std::string & file, const std::string & e) throw () :
    Exception("Cannot dlopen an environment .so file"),
    _file(file),
    _dlerr(e)
{
}

PaludisEnvironmentSoDirCannotDlopenError::~PaludisEnvironmentSoDirCannotDlopenError() throw ()
{
}

const char *
PaludisEnvironmentSoDirCannotDlopenError::what() const throw ()
{
    if (_what.empty())
        _what = std::string(Exception::what()) +
            ": Cannot dlopen environment .so file '" + _file + "': '" + _dlerr + "'";
    return _what.c_str();
}

namespace paludis
{
    template<>
    struct Implementation<EnvironmentMaker>
    {
        std::list<void *> dl_opened;
    };
}

void
EnvironmentMaker::load_dir(const FSEntry & so_dir)
{
    for (DirIterator d(so_dir), d_end ; d != d_end ; ++d)
    {
        if (d->is_directory())
            load_dir(*d);

        if (! is_file_with_extension(*d, ".so." + stringify(100 * PALUDIS_VERSION_MAJOR + PALUDIS_VERSION_MINOR),
                    IsFileWithOptions()))
            continue;

        /* don't use RTLD_LOCAL, g++ is over happy about template instantiations, and it
         * can lead to multiple singleton instances. */
        void * dl(dlopen(stringify(*d).c_str(), RTLD_GLOBAL | RTLD_NOW));

        if (dl)
        {
            _imp->dl_opened.push_back(dl);

            void * reg(dlsym(dl, "register_environments"));
            if (reg)
            {
                reinterpret_cast<void (*)(EnvironmentMaker *)>(
                        reinterpret_cast<uintptr_t>(reg))(this);
            }
            else
                throw PaludisEnvironmentSoDirCannotDlopenError(stringify(*d),
                        "no register_environments function defined");
        }
        else
            throw PaludisEnvironmentSoDirCannotDlopenError(stringify(*d), dlerror());
    }

    if ((so_dir / ".libs").is_directory())
        load_dir(so_dir / ".libs");
}

EnvironmentMaker::EnvironmentMaker() :
    PrivateImplementationPattern<EnvironmentMaker>(new Implementation<EnvironmentMaker>)
{
    FSEntry so_dir(getenv_with_default("PALUDIS_ENVIRONMENT_SO_DIR", LIBDIR "/paludis/environments"));

    if (! so_dir.is_directory())
        throw PaludisEnvironmentSoDirNotADirectoryError();

    load_dir(so_dir);
}

EnvironmentMaker::~EnvironmentMaker()
{
}

tr1::shared_ptr<Environment>
EnvironmentMaker::make_from_spec(const std::string & s) const
{
    Context context("When making environment from specification '" + s + "':");

    std::string key, suffix;
    std::string::size_type p(s.find(':'));

    if (std::string::npos == p)
        key = s;
    else
    {
        key = s.substr(0, p);
        suffix = s.substr(p + 1);
    }

    if (key.empty())
        key = DistributionData::get_instance()->default_distribution()->default_environment;

    try
    {
        return (*find_maker(key))(suffix);
    }
    catch (const FallBackToAnotherMakerError &)
    {
        std::string f(DistributionData::get_instance()->default_distribution()->fallback_environment);
        if (s.empty() && ! f.empty())
        {
            std::set<std::string> keys;
            copy_keys(std::inserter(keys, keys.begin()));
            if (keys.end() != keys.find(f))
                return make_from_spec(f);
            else
                throw;
        }
        else
            throw;
    }
}

FallBackToAnotherMakerError::FallBackToAnotherMakerError()
{
}

extern "C"
{
    void PALUDIS_VISIBLE register_environments(EnvironmentMaker * maker);
}

void register_environments(EnvironmentMaker *)
{
}

