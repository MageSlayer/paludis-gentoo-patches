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

#include "vdb_unmerger.hh"

using namespace paludis;

#include <paludis/repositories/gentoo/vdb_unmerger-sr.cc>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/digests/md5.hh>
#include <list>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace paludis
{
    template<>
    struct Implementation<VDBUnmerger>
    {
        VDBUnmergerOptions options;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Implementation(const VDBUnmergerOptions & o) :
            options(o)
        {
            WhitespaceTokeniser::get_instance()->tokenise(o.config_protect,
                    std::back_inserter(config_protect));
            WhitespaceTokeniser::get_instance()->tokenise(o.config_protect_mask,
                    std::back_inserter(config_protect_mask));
        }
    };
}

VDBUnmerger::VDBUnmerger(const VDBUnmergerOptions & o) :
    PrivateImplementationPattern<VDBUnmerger>(new Implementation<VDBUnmerger>(o))
{
}

VDBUnmerger::~VDBUnmerger()
{
}

void
VDBUnmerger::unmerge()
{
    std::ifstream c(stringify(_imp->options.contents_file).c_str());
    if (! c)
        throw VDBUnmergerError("Cannot read '" + stringify(_imp->options.contents_file) + "'");

    std::list<std::string> lines;
    std::string line;
    while (std::getline(c, line))
        lines.push_back(line);

    unmerge_non_directories(lines.begin(), lines.end());
    unmerge_directories(lines.rbegin(), lines.rend());
}

bool
VDBUnmerger::config_protected(const FSEntry & f)
{
    std::string tidy(make_tidy(f));

    bool result(false);
    for (std::list<std::string>::const_iterator c(_imp->config_protect.begin()),
            c_end(_imp->config_protect.end()) ; c != c_end && ! result ; ++c)
        if (0 == tidy.compare(0, c->length(), *c))
            result = true;
    if (result)
        for (std::list<std::string>::const_iterator c(_imp->config_protect_mask.begin()),
                c_end(_imp->config_protect_mask.end()) ; c != c_end && result ; ++c)
            if (0 == tidy.compare(0, c->length(), *c))
                result = false;

    return result;
}

std::string
VDBUnmerger::make_tidy(const FSEntry & f) const
{
    std::string root_str(stringify(_imp->options.root)), f_str(stringify(f));
    if (root_str == "/")
        root_str.clear();
    if (0 != f_str.compare(0, root_str.length(), root_str))
        throw VDBUnmergerError("Can't work out tidy name for '" + f_str + "' with root '" + root_str + "'");
    return f_str.substr(root_str.length());
}

template <typename I_>
void
VDBUnmerger::unmerge_non_directories(I_ cur, const I_ end)
{
    for ( ; cur != end ; ++cur)
    {
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*cur, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if ("obj" == tokens.at(0))
        {
            while (tokens.size() > 4)
            {
                if (std::string::npos != tokens.at(4).find('='))
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                for (unsigned i = 2 ; i < tokens.size() - 1 ; ++i)
                    tokens.at(i) = tokens.at(i + 1);
                tokens.pop_back();
            }

            if (tokens.size() != 4)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + *cur + "'");
            else if (! (_imp->options.root / tokens.at(1)).is_regular_file())
                std::cout << "--- [!type] " << tokens.at(1) << std::endl;
            else if (stringify((_imp->options.root / tokens.at(1)).mtime()) != tokens.at(3))
                std::cout << "--- [!time] " << tokens.at(1) << std::endl;
            else
            {
                std::ifstream md5_file(stringify(_imp->options.root / tokens.at(1)).c_str());
                if (! md5_file)
                {
                    Log::get_instance()->message(ll_warning, lc_no_context, "Cannot get md5 for '" +
                            stringify(_imp->options.root / tokens.at(1)) + "'");
                    std::cout << "--- [!md5?] " << tokens.at(1) << std::endl;
                }
                else if (MD5(md5_file).hexsum() != tokens.at(2))
                    std::cout << "--- [!md5 ] " << tokens.at(1) << std::endl;
                else if (config_protected(_imp->options.root / tokens.at(1)))
                    std::cout << "--- [cfgpr] " << tokens.at(1) << std::endl;
                else
                {
                    std::cout << "<<<         " << tokens.at(1) << std::endl;
                    mode_t mode((_imp->options.root / tokens.at(1)).permissions());
                    if ((mode & S_ISUID) || (mode & S_ISGID))
                    {
                        mode &= 0400;
                        (_imp->options.root / tokens.at(1)).chmod(mode);
                    }
                    (_imp->options.root / tokens.at(1)).unlink();
                }
            }
        }
        else if ("sym" == tokens.at(0))
        {
            while (tokens.size() > 5)
            {
                if (std::string::npos != tokens.at(2).find('='))
                    break;

                if (tokens.at(2) == "->")
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                    tokens.at(i) = tokens.at(i + 1);
                tokens.pop_back();
            }

            while (tokens.size() > 5)
            {
                if (std::string::npos != tokens.at(2).find('='))
                    break;

                if (tokens.at(4) == "->")
                    break;

                tokens.at(3).append(" " + tokens.at(4));
                for (unsigned i = 4 ; i < tokens.size() - 1; ++i)
                    tokens.at(i) = tokens.at(i + 1);
                tokens.pop_back();
            }

            if (tokens.size() != 5)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + *cur + "'");
            else if (! (_imp->options.root / tokens.at(1)).is_symbolic_link())
                std::cout << "--- [!type] " << tokens.at(1) << std::endl;
            else if (stringify((_imp->options.root / tokens.at(1)).mtime()) != tokens.at(4))
                std::cout << "--- [!time] " << tokens.at(1) << std::endl;
            else if ((_imp->options.root / tokens.at(1)).readlink() != tokens.at(3))
                std::cout << "--- [!dest] " << tokens.at(1) << std::endl;
            else
            {
                std::cout << "<<<         " << tokens.at(1) << std::endl;
                (_imp->options.root / tokens.at(1)).unlink();
            }
        }
        else if ("misc" == tokens.at(0))
        {
        }
        else if ("fif" == tokens.at(0) || "dev" == tokens.at(0))
        {
            while (tokens.size() > 2)
            {
                if (std::string::npos != tokens.at(2).find('='))
                    break;

                tokens.at(1).append(" " + tokens.at(2));
                for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                    tokens.at(i) = tokens.at(i + 1);
                tokens.pop_back();
            }

            if (tokens.size() != 2)
                Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + *cur + "'");
            else if ("fif" == tokens.at(0) && ! (_imp->options.root / tokens.at(1)).is_fifo())
                std::cout << "--- [!type] " << tokens.at(1) << std::endl;
            else if ("dev" == tokens.at(0) && ! (_imp->options.root / tokens.at(1)).is_device())
                std::cout << "--- [!type] " << tokens.at(1) << std::endl;
            else
            {
                std::cout << "<<<         " << tokens.at(1) << std::endl;
                (_imp->options.root / tokens.at(1)).unlink();
            }
        }
        else if ("dir" == tokens.at(0))
            /* nothing */ ;
        else
            Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + *cur + "'");
    }
}

template <typename I_>
void
VDBUnmerger::unmerge_directories(I_ cur, const I_ end)
{
    for ( ; cur != end ; ++cur)
    {
        std::vector<std::string> tokens;
        WhitespaceTokeniser::get_instance()->tokenise(*cur, std::back_inserter(tokens));
        if (tokens.empty())
            continue;

        if ("dir" != tokens.at(0))
            continue;

        while (tokens.size() > 2)
        {
            if (std::string::npos != tokens.at(2).find('='))
                break;

            tokens.at(1).append(" " + tokens.at(2));
            for (unsigned i = 2 ; i < tokens.size() - 1; ++i)
                tokens.at(i) = tokens.at(i + 1);
            tokens.pop_back();
        }

        if (tokens.size() != 2)
            Log::get_instance()->message(ll_warning, lc_no_context, "Malformed VDB entry '" + *cur + "'");
        else if (! (_imp->options.root / tokens.at(1)).is_directory())
            std::cout << "--- [!type] " << tokens.at(1) << std::endl;
        else if (DirIterator(_imp->options.root / tokens.at(1), false) != DirIterator())
            std::cout << "--- [!empt] " << tokens.at(1) << std::endl;
        else
        {
            std::cout << "<<<         " << tokens.at(1) << std::endl;
            (_imp->options.root / tokens.at(1)).rmdir();
        }
    }
}

VDBUnmergerError::VDBUnmergerError(const std::string & s) throw () :
    Exception(s)
{
}

