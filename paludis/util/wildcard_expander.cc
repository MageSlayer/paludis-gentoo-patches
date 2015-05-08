/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/util/wildcard_expander.hh>

#include <paludis/util/exception.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <memory>
#include <vector>

#include <cerrno>
#include <cstring>
#include <glob.h>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<WildcardExpander>
    {
        std::shared_ptr<std::vector<FSPath> > files;
        std::vector<FSPath>::const_iterator it;

        Imp() :
            files(std::make_shared<std::vector<FSPath>>())
        {
        }

        Imp(const std::shared_ptr<std::vector<FSPath> > & the_files,
                       std::vector<FSPath>::const_iterator the_it) :
            files(the_files),
            it(the_it)
        {
        }
    };
}

WildcardExpansionError::WildcardExpansionError(const std::string & the_message) noexcept :
    FSError(the_message)
{
}

namespace
{
    struct Glob
    {
        glob_t g;

        Glob(const std::string &, int);

        const glob_t & operator* ()
        {
            return g;
        }

        const glob_t * operator-> ()
        {
            return &g;
        }

        ~Glob()
        {
            globfree(&g);
        }
    };

    Glob::Glob(const std::string & pattern, int flags)
    {
        int ret(glob(pattern.c_str(), flags, nullptr, &g));
        switch (ret)
        {
            case 0:
            case GLOB_NOMATCH:
                break;

            case GLOB_NOSPACE:
                globfree(&g);
                throw std::bad_alloc();

            case GLOB_ABORTED:
                globfree(&g);
                throw WildcardExpansionError("Error running glob(): " + stringify(std::strerror(errno)));

            default:
                globfree(&g);
                throw InternalError(PALUDIS_HERE, "glob() returned unrecognised value " + stringify(ret));
        }
    }
}

WildcardExpander::WildcardExpander(const std::string & pattern, const FSPath & root) :
    _imp()
{
    // GLOB_NOCHECK seems to be buggy in glibc 2.5: fails
    //        TEST_CHECK_EQUAL(expand("/foo\\*"), "/foo*");
    // with
    //        Expected 'expand("/foo\\*")' to equal '/foo*' but got '/foo\*'
    Glob g(stringify(root / pattern), 0);
    if (0 < g->gl_pathc)
        for (unsigned i = 0; i < g->gl_pathc; ++i)
            _imp->files->push_back(FSPath(g->gl_pathv[i]).strip_leading(root));
    else
        _imp->files->push_back(FSPath("/") / pattern);

    _imp->it = _imp->files->begin();
}

WildcardExpander::WildcardExpander() :
    _imp()
{
    _imp->it = _imp->files->end();
}

WildcardExpander::WildcardExpander(const WildcardExpander & other) :
    _imp(other._imp->files, other._imp->it)
{
}

WildcardExpander::~WildcardExpander()
{
}

WildcardExpander &
WildcardExpander::operator= (const WildcardExpander & other)
{
    _imp->files = other._imp->files;
    _imp->it = other._imp->it;
    return *this;
}

const FSPath &
WildcardExpander::operator* () const
{
    return *_imp->it;
}

const FSPath *
WildcardExpander::operator-> () const
{
    return &*_imp->it;
}

WildcardExpander &
WildcardExpander::operator++ ()
{
    ++_imp->it;
    return *this;
}

WildcardExpander
WildcardExpander::operator++ (int)
{
    WildcardExpander copy(*this);
    ++_imp->it;
    return copy;
}

bool
WildcardExpander::operator== (const WildcardExpander & other) const
{
    if (_imp->it == _imp->files->end() && other._imp->it == other._imp->files->end())
        return true;
    return _imp->files == other._imp->files && _imp->it == other._imp->it;
}

namespace paludis
{
    template class Pimp<WildcardExpander>;
}
