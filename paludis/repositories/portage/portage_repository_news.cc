/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/repositories/portage/portage_repository_news.hh>

#include <paludis/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>

#include <set>
#include <ostream>
#include <fstream>
#include <list>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PortageRepositoryNews> :
        InternalCounted<Implementation<PortageRepositoryNews> >
    {
        const Environment * const environment;
        const PortageRepository * const portage_repository;
        const PortageRepositoryParams params;

        const FSEntry skip_file;
        const FSEntry unread_file;

        Implementation(const Environment * const e, const PortageRepository * const p,
                const PortageRepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k),
            skip_file(params.get<prpk_root>() / "var" / "lib" / "paludis" / "news" /
                    ("news-" + stringify(portage_repository->name()) + ".skip")),
            unread_file(params.get<prpk_root>() / "var" / "lib" / "paludis" / "news" /
                    ("news-" + stringify(portage_repository->name()) + ".unread"))
        {
        }
    };
}

PortageRepositoryNews::PortageRepositoryNews(const Environment * const e, const PortageRepository * const p,
        const PortageRepositoryParams & k) :
    PrivateImplementationPattern<PortageRepositoryNews>(new Implementation<PortageRepositoryNews>(e, p, k))
{
}

PortageRepositoryNews::~PortageRepositoryNews()
{
}

void
PortageRepositoryNews::update_news() const
{
    Context context("When updating news at location '" +
            stringify(_imp->params.get<prpk_newsdir>()) + "' for repository '" +
            stringify(_imp->portage_repository->name()) + "':");

    if (! _imp->params.get<prpk_newsdir>().is_directory())
        return;

    std::set<std::string> skip;

    if (_imp->skip_file.is_regular_file())
    {
        Context local_context("When handling news skip file '" + stringify(
                _imp->skip_file) + "':");
        LineConfigFile s(_imp->skip_file);
        std::copy(s.begin(), s.end(), std::inserter(skip, skip.end()));
    }

    for (DirIterator d(_imp->params.get<prpk_newsdir>()), d_end ; d != d_end ; ++d)
    {
        Context local_context("When handling news entry '" + stringify(*d) + "':");

        if (! d->is_directory())
            continue;
        if (! (*d / (d->basename() + ".en.txt")).is_regular_file())
            continue;

        if (skip.end() != skip.find(d->basename()))
            continue;

        try
        {
            NewsFile news(*d / (d->basename() + ".en.txt"));
            bool show(true);

            if (news.begin_display_if_installed() != news.end_display_if_installed())
            {
                bool local_show(false);
                for (NewsFile::DisplayIfInstalledIterator i(news.begin_display_if_installed()),
                        i_end(news.end_display_if_installed()) ; i != i_end ; ++i)
                    if (! _imp->environment->package_database()->query(PackageDepAtom::Pointer(
                                    new PackageDepAtom(*i)), is_installed_only)->empty())
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_keyword() != news.end_display_if_keyword())
            {
                bool local_show(false);
                for (NewsFile::DisplayIfKeywordIterator i(news.begin_display_if_keyword()),
                        i_end(news.end_display_if_keyword()) ; i != i_end && ! local_show ; ++i)
                    if (*i == _imp->portage_repository->profile_variable("ARCH"))
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_profile() != news.end_display_if_profile())
            {
                bool local_show(false);
                FSEntryCollection::ConstPointer c(_imp->params.get<prpk_profiles>());
                for (FSEntryCollection::Iterator p(c->begin()), p_end(c->end()) ; p != p_end ; ++p)
                {
                    std::string profile(strip_leading_string(strip_trailing_string(
                                strip_leading_string(stringify(p->realpath()),
                                    stringify(p->realpath())), "/"), "/"));
                    Log::get_instance()->message(ll_debug, lc_no_context,
                            "Profile path is '" + profile + "'");
                    for (NewsFile::DisplayIfProfileIterator i(news.begin_display_if_profile()),
                            i_end(news.end_display_if_profile()) ; i != i_end ; ++i)
                        if (profile == *i)
                            local_show = true;
                }
                show &= local_show;
            }

            if (show)
            {
                std::ofstream s(stringify(_imp->skip_file).c_str(),
                        std::ios::out | std::ios::app);
                if (! s)
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Cannot append to news skip file '"
                            + stringify(_imp->skip_file) +
                            "', skipping news item '" + stringify(*d) + "'");

                std::ofstream t(stringify(_imp->unread_file).c_str(),
                        std::ios::out | std::ios::app);
                if (! t)
                    Log::get_instance()->message(ll_warning, lc_no_context,
                            "Cannot append to unread file '"
                            + stringify(_imp->unread_file) +
                            "', skipping news item '" + stringify(*d) + "'");

                if (s && t)
                {
                    s << d->basename() << std::endl;
                    t << d->basename() << std::endl;
                }
            }
        }
        catch (const ConfigFileError & e)
        {
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "Skipping news item '"
                    + stringify(*d) + "' because of exception '" + e.message() + "' ("
                    + e.what() + ")");
        }
    }

}

namespace paludis
{
    template<>
    struct Implementation<NewsFile> :
        InternalCounted<Implementation<NewsFile> >
    {
        mutable bool in_header;
        mutable std::list<std::string> display_if_installed;
        mutable std::list<std::string> display_if_keyword;
        mutable std::list<std::string> display_if_profile;

        Implementation() :
            in_header(true)
        {
        }
    };
}

NewsFile::NewsFile(const FSEntry & filename) :
    ConfigFile(filename),
    PrivateImplementationPattern<NewsFile>(new Implementation<NewsFile>)
{
    need_lines();
}

NewsFile::~NewsFile()
{
}

void
NewsFile::accept_line(const std::string & line) const
{
    if (_imp->in_header)
    {
        std::string::size_type p(line.find(':'));
        if (std::string::npos == p)
            _imp->in_header = false;
        else
        {
            std::string k(strip_leading(strip_trailing(line.substr(0, p), " \t\n"), " \t\n"));
            std::string v(strip_leading(strip_trailing(line.substr(p + 1), " \t\n"), " \t\n"));
            if (k == "Display-If-Installed")
                _imp->display_if_installed.push_back(v);
            else if (k == "Display-If-Keyword")
                _imp->display_if_keyword.push_back(v);
            if (k == "Display-If-Profile")
                _imp->display_if_profile.push_back(v);
        }
    }
}

NewsFile::DisplayIfInstalledIterator
NewsFile::begin_display_if_installed() const
{
    return DisplayIfInstalledIterator(_imp->display_if_installed.begin());
}

NewsFile::DisplayIfInstalledIterator
NewsFile::end_display_if_installed() const
{
    return DisplayIfInstalledIterator(_imp->display_if_installed.end());
}

NewsFile::DisplayIfKeywordIterator
NewsFile::begin_display_if_keyword() const
{
    return DisplayIfKeywordIterator(_imp->display_if_keyword.begin());
}

NewsFile::DisplayIfKeywordIterator
NewsFile::end_display_if_keyword() const
{
    return DisplayIfKeywordIterator(_imp->display_if_keyword.end());
}

NewsFile::DisplayIfProfileIterator
NewsFile::begin_display_if_profile() const
{
    return DisplayIfProfileIterator(_imp->display_if_profile.begin());
}

NewsFile::DisplayIfProfileIterator
NewsFile::end_display_if_profile() const
{
    return DisplayIfProfileIterator(_imp->display_if_profile.end());
}


