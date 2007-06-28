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

#include <paludis/repositories/e/portage_repository.hh>
#include <paludis/repositories/e/portage_repository_news.hh>

#include <paludis/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

#include <set>
#include <ostream>
#include <fstream>
#include <list>

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for PortageRepositoryNews.
     *
     * \ingroup grpportagerepository
     */
    template<>
    struct Implementation<PortageRepositoryNews>
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
            skip_file(e->root() / "var" / "lib" / "paludis" / "news" /
                    ("news-" + stringify(portage_repository->name()) + ".skip")),
            unread_file(e->root() / "var" / "lib" / "paludis" / "news" /
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
            stringify(_imp->params.newsdir) + "' for repository '" +
            stringify(_imp->portage_repository->name()) + "':");

    if (! _imp->params.newsdir.is_directory_or_symlink_to_directory())
        return;

    std::set<std::string> skip;

    if (_imp->skip_file.is_regular_file_or_symlink_to_regular_file())
    {
        Context local_context("When handling news skip file '" + stringify(
                _imp->skip_file) + "':");
        LineConfigFile s(_imp->skip_file, LineConfigFileOptions());
        std::copy(s.begin(), s.end(), std::inserter(skip, skip.end()));
    }

    for (DirIterator d(_imp->params.newsdir), d_end ; d != d_end ; ++d)
    {
        Context local_context("When handling news entry '" + stringify(*d) + "':");

        if (0 == stringify(_imp->portage_repository->name()).compare(0, 2, "x-"))
        {
            Log::get_instance()->message(ll_warning, lc_context, "Cannot enable GLEP 42 news items for repository '"
                    + stringify(_imp->portage_repository->name()) + "' because it is using a generated repository name");
            return;
        }

        if (! d->is_directory_or_symlink_to_directory())
            continue;
        if (! (*d / (d->basename() + ".en.txt")).is_regular_file_or_symlink_to_regular_file())
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
                    if (! _imp->environment->package_database()->query(
                                query::Matches(PackageDepSpec(*i, pds_pm_permissive)) &
                                query::RepositoryHasInstalledInterface(),
                                qo_whatever)->empty())
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
                tr1::shared_ptr<const FSEntryCollection> c(_imp->params.profiles);
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
        catch (const Exception & e)
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
    /**
     * Implementation data for NewsFile.
     *
     * \ingroup grpnewsconfigfile
     */
    template<>
    struct Implementation<NewsFile>
    {
        mutable std::list<std::string> display_if_installed;
        mutable std::list<std::string> display_if_keyword;
        mutable std::list<std::string> display_if_profile;
    };
}

NewsFile::NewsFile(const FSEntry & our_filename) :
    PrivateImplementationPattern<NewsFile>(new Implementation<NewsFile>)
{
    Context context("When parsing GLEP 42 news file '" + stringify(our_filename) + "':");

    LineConfigFile line_file(our_filename, LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_no_skip_blank_lines
            + lcfo_disallow_comments);
    for (LineConfigFile::Iterator line(line_file.begin()), line_end(line_file.end()) ;
            line != line_end ; ++line)
    {
        if (line->empty())
            break;

        std::string::size_type p(line->find(':'));
        if (std::string::npos == p)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Bad header line '" + *line + "'");
            break;
        }
        else
        {
            std::string k(strip_leading(strip_trailing(line->substr(0, p), " \t\n"), " \t\n"));
            std::string v(strip_leading(strip_trailing(line->substr(p + 1), " \t\n"), " \t\n"));
            if (k == "Display-If-Installed")
                _imp->display_if_installed.push_back(v);
            else if (k == "Display-If-Keyword")
                _imp->display_if_keyword.push_back(v);
            if (k == "Display-If-Profile")
                _imp->display_if_profile.push_back(v);
        }
    }
}

NewsFile::~NewsFile()
{
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


