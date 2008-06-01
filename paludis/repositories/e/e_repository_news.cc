/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_news.hh>
#include <paludis/repositories/e/eapi.hh>

#include <paludis/util/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>

#include <set>
#include <ostream>
#include <fstream>
#include <list>

using namespace paludis;

template class WrappedForwardIterator<NewsFile::DisplayIfInstalledConstIteratorTag, const std::string>;
template class WrappedForwardIterator<NewsFile::DisplayIfKeywordConstIteratorTag, const std::string>;
template class WrappedForwardIterator<NewsFile::DisplayIfProfileConstIteratorTag, const std::string>;

namespace paludis
{
    /**
     * Implementation data for ERepositoryNews.
     *
     * \ingroup grperepository
     */
    template<>
    struct Implementation<ERepositoryNews>
    {
        const Environment * const environment;
        const ERepository * const e_repository;
        const ERepositoryParams params;

        const FSEntry skip_file;
        const FSEntry unread_file;

        Implementation(const Environment * const e, const ERepository * const p,
                const ERepositoryParams & k) :
            environment(e),
            e_repository(p),
            params(k),
            skip_file(e->root() / "var" / "lib" / "gentoo" / "news" /
                    ("news-" + stringify(e_repository->name()) + ".skip")),
            unread_file(e->root() / "var" / "lib" / "gentoo" / "news" /
                    ("news-" + stringify(e_repository->name()) + ".unread"))
        {
        }
    };
}

ERepositoryNews::ERepositoryNews(const Environment * const e, const ERepository * const p,
        const ERepositoryParams & k) :
    PrivateImplementationPattern<ERepositoryNews>(new Implementation<ERepositoryNews>(e, p, k))
{
}

ERepositoryNews::~ERepositoryNews()
{
}

void
ERepositoryNews::update_news() const
{
    Context context("When updating news at location '" +
            stringify(_imp->params.newsdir) + "' for repository '" +
            stringify(_imp->e_repository->name()) + "':");

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

        if (0 == stringify(_imp->e_repository->name()).compare(0, 2, "x-"))
        {
            Log::get_instance()->message("e.news.no_repo_name", ll_warning, lc_context)
                << "Cannot enable GLEP 42 news items for repository '"
                << _imp->e_repository->name() << "' because it is using a generated repository name";
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
                for (NewsFile::DisplayIfInstalledConstIterator i(news.begin_display_if_installed()),
                        i_end(news.end_display_if_installed()) ; i != i_end ; ++i)
                    if (! (*_imp->environment)[selection::SomeArbitraryVersion(
                                generator::Matches(PackageDepSpec(parse_elike_package_dep_spec(*i,
                                            (*(*erepository::EAPIData::get_instance()->eapi_from_string(
                                                _imp->e_repository->params().profile_eapi))[k::supported()])[k::package_dep_spec_parse_options()],
                                            std::tr1::shared_ptr<const PackageID>()))) |
                                filter::SupportsAction<InstalledAction>())]->empty())
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_keyword() != news.end_display_if_keyword())
            {
                bool local_show(false);
                for (NewsFile::DisplayIfKeywordConstIterator i(news.begin_display_if_keyword()),
                        i_end(news.end_display_if_keyword()) ; i != i_end && ! local_show ; ++i)
                    if (*i == _imp->e_repository->profile_variable("ARCH"))
                        local_show = true;
                show &= local_show;
            }

            if (news.begin_display_if_profile() != news.end_display_if_profile())
            {
                bool local_show(false);
                std::tr1::shared_ptr<const FSEntrySequence> c(_imp->params.profiles);
                for (FSEntrySequence::ConstIterator p(c->begin()), p_end(c->end()) ; p != p_end ; ++p)
                {
                    std::string profile(strip_leading_string(strip_trailing_string(
                                strip_leading_string(stringify(p->realpath()),
                                    stringify(p->realpath())), "/"), "/"));
                    Log::get_instance()->message("e.news.profile_path", ll_debug, lc_no_context) <<
                        "Profile path is '" << profile << "'";
                    for (NewsFile::DisplayIfProfileConstIterator i(news.begin_display_if_profile()),
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
                    Log::get_instance()->message("e.news.skip_file.append_failure", ll_warning, lc_no_context) <<
                        "Cannot append to news skip file '" << _imp->skip_file <<
                        "', skipping news item '" << *d << "'";

                std::ofstream t(stringify(_imp->unread_file).c_str(),
                        std::ios::out | std::ios::app);
                if (! t)
                    Log::get_instance()->message("e.news.unread_file.append_failure", ll_warning, lc_no_context) <<
                        "Cannot append to unread file '" << _imp->unread_file <<
                        "', skipping news item '" << *d << "'";

                if (s && t)
                {
                    s << d->basename() << std::endl;
                    t << d->basename() << std::endl;
                }
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("e.news.skipping", ll_warning, lc_no_context) <<
                "Skipping news item '" << *d << "' because of exception '" << e.message() << "' ("
                << e.what() << ")";
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
        std::list<std::string> display_if_installed;
        std::list<std::string> display_if_keyword;
        std::list<std::string> display_if_profile;
    };
}

NewsFile::NewsFile(const FSEntry & our_filename) :
    PrivateImplementationPattern<NewsFile>(new Implementation<NewsFile>)
{
    Context context("When parsing GLEP 42 news file '" + stringify(our_filename) + "':");

    LineConfigFile line_file(our_filename, LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_no_skip_blank_lines
            + lcfo_disallow_comments);
    for (LineConfigFile::ConstIterator line(line_file.begin()), line_end(line_file.end()) ;
            line != line_end ; ++line)
    {
        if (line->empty())
            break;

        std::string::size_type p(line->find(':'));
        if (std::string::npos == p)
        {
            Log::get_instance()->message("e.news.bad_header", ll_warning, lc_context)
                << "Bad header line '" << *line << "'";
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

NewsFile::DisplayIfInstalledConstIterator
NewsFile::begin_display_if_installed() const
{
    return DisplayIfInstalledConstIterator(_imp->display_if_installed.begin());
}

NewsFile::DisplayIfInstalledConstIterator
NewsFile::end_display_if_installed() const
{
    return DisplayIfInstalledConstIterator(_imp->display_if_installed.end());
}

NewsFile::DisplayIfKeywordConstIterator
NewsFile::begin_display_if_keyword() const
{
    return DisplayIfKeywordConstIterator(_imp->display_if_keyword.begin());
}

NewsFile::DisplayIfKeywordConstIterator
NewsFile::end_display_if_keyword() const
{
    return DisplayIfKeywordConstIterator(_imp->display_if_keyword.end());
}

NewsFile::DisplayIfProfileConstIterator
NewsFile::begin_display_if_profile() const
{
    return DisplayIfProfileConstIterator(_imp->display_if_profile.begin());
}

NewsFile::DisplayIfProfileConstIterator
NewsFile::end_display_if_profile() const
{
    return DisplayIfProfileConstIterator(_imp->display_if_profile.end());
}


