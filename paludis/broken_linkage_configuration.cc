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

#include <paludis/broken_linkage_configuration.hh>
#include <paludis/util/realpath.hh>
#include <paludis/util/wildcard_expander.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/options.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_stat.hh>
#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>
#include <cctype>

using namespace paludis;

namespace paludis
{
    template <>
    struct Imp<BrokenLinkageConfiguration>
    {
        std::vector<std::string> ld_library_mask;
        std::vector<FSPath> search_dirs;
        std::vector<FSPath> search_dirs_mask;
        std::vector<FSPath> ld_so_conf;

        void load_from_environment();
        void load_from_etc_revdep_rebuild(const FSPath &);
        void load_from_etc_profile_env(const FSPath &);
        void load_from_etc_ld_so_conf(const FSPath &);
        void add_defaults();
    };

    template <>
    struct WrappedForwardIteratorTraits<BrokenLinkageConfiguration::DirsIteratorTag>
    {
        typedef std::vector<FSPath>::const_iterator UnderlyingIterator;
    };
}

namespace
{
    template <typename T_>
    void
    from_colon_string(const std::function<std::string (const std::string &)> & source,
                const std::string & varname, std::vector<T_> & vec)
    {
        std::string str(source(varname));
        if (! str.empty())
        {
            Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
                << "Got " << varname << "=\"" + str << "\"";
            tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(str, ":", "", create_inserter<T_>(std::back_inserter(vec)));
        }
    }

    template <typename T_>
    void
    from_string(const std::function<std::string (const std::string &)> & source,
                const std::string & varname, std::vector<T_> & vec)
    {
        std::string str(source(varname));
        if (! str.empty())
        {
            Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
                << "Got " << varname << "=\"" << str << "\"";
            tokenise_whitespace(str, create_inserter<T_>(std::back_inserter(vec)));
        }
    }

    inline void
    do_wildcards(std::vector<std::string> &, const FSPath &)
    {
    }

    inline void
    do_wildcards(std::vector<FSPath> & vec, const FSPath & root)
    {
        std::vector<FSPath> scratch;

        for (const auto & path : vec)
            std::copy(WildcardExpander(stringify(path), root), WildcardExpander(),
                      std::back_inserter(scratch));

        std::swap(vec, scratch);
    }

    template <typename T_, typename C_>
    void
    cleanup(const std::string & varname, std::vector<T_> & vec, const FSPath & root, const C_ & comparator)
    {
        vec.erase(std::find(vec.begin(), vec.end(), T_("-*")), vec.end());

        do_wildcards(vec, root);

        std::sort(vec.begin(), vec.end(), comparator);
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());

        Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
            << "Final " << varname << "=\"" << join(vec.begin(), vec.end(),  " ") << "\"";
    }

    bool
    char_equals_ci(char a, char b)
    {
        return std::tolower(a) == std::tolower(b);
    }

    bool
    equals_ci(const std::string & a, const std::string & b)
    {
        return a.length() == b.length() && std::equal(a.begin(), a.end(), b.begin(), char_equals_ci);
    }

    void
    parse_ld_so_conf(const FSPath & root, const FSPath & file, const LineConfigFileOptions & opts, std::vector<std::string> & res)
    {
        FSStat file_stat(file);

        if (file_stat.is_regular_file_or_symlink_to_regular_file())
        {
            LineConfigFile lines(file, opts);
            for (const auto & line : lines)
            {
                std::vector<std::string> tokens;
                tokenise_whitespace(line, std::back_inserter(tokens));

                if ("include" == tokens.at(0))
                {
                    for (const auto & token : tokens)
                    {
                        FSPath rel('/' == token.at(0) ? root : file.dirname());
                        for (WildcardExpander it3(token, rel), it3_end; it3_end != it3; ++it3)
                        {
                            Context ctx("When reading included file '" + stringify(rel / *it3) + "':");
                            parse_ld_so_conf(root, rel / *it3, opts, res);
                        }
                    }
                }
                else if (equals_ci("hwdep", tokens.at(0))) // XXX
                {
                    Log::get_instance()->message("broken_linkage_finder.etc_ld_so_conf.hwdep", ll_warning, lc_context)
                        << "'hwdep' line in '" << file << "' is not supported";
                }
                else if (std::string::npos != line.find('='))
                {
                    Log::get_instance()->message("broken_linkage_finder.etc_ld_so_conf.equals", ll_warning, lc_context)
                        << "'=' line in '" << file << "' is not supported";
                }
                else
                {
                    res.push_back(line);
                }
            }
        }

        else if (file_stat.exists())
            Log::get_instance()->message("broken_linkage_finder.etc_ld_so_conf.not_a_file", ll_warning, lc_context)
                << "'" << file << "' exists but is not a regular file";
    }
}

BrokenLinkageConfiguration::BrokenLinkageConfiguration(const FSPath & root) :
    _imp()
{
    Context ctx("When loading broken linkage checker configuration for '" + stringify(root) + "':");

    _imp->load_from_environment();
    _imp->load_from_etc_revdep_rebuild(root);
    _imp->load_from_etc_profile_env(root);
    _imp->load_from_etc_ld_so_conf(root);
    _imp->add_defaults();

    cleanup("LD_LIBRARY_MASK",  _imp->ld_library_mask,  root, std::less<>());
    cleanup("SEARCH_DIRS",      _imp->search_dirs,      root, FSPathComparator());
    cleanup("SEARCH_DIRS_MASK", _imp->search_dirs_mask, root, FSPathComparator());

    // don't need the extra cleanup here
    std::sort(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end(), FSPathComparator());
    _imp->ld_so_conf.erase(std::unique(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end()),
                           _imp->ld_so_conf.end());
    Log::get_instance()->message("broken_linkage_finder.config",
            ll_debug, lc_context) << "Final ld.so.conf contents is \"" <<
        join(_imp->ld_so_conf.begin(), _imp->ld_so_conf.end(), " ") << "\"";
}

BrokenLinkageConfiguration::~BrokenLinkageConfiguration() = default;

void
Imp<BrokenLinkageConfiguration>::load_from_environment()
{
    using namespace std::placeholders;

    Context ctx("When checking environment variables:");

    std::function<std::string (const std::string &)> fromenv(std::bind(getenv_with_default, _1, ""));

    from_string(fromenv, "LD_LIBRARY_MASK",  ld_library_mask);
    from_string(fromenv, "SEARCH_DIRS",      search_dirs);
    from_string(fromenv, "SEARCH_DIRS_MASK", search_dirs_mask);
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_revdep_rebuild(const FSPath & root)
{
    using namespace std::placeholders;

    FSPath etc_revdep_rebuild(root / "etc" / "revdep-rebuild");
    FSStat etc_revdep_rebuild_stat(etc_revdep_rebuild);
    Context ctx("When reading '" + stringify(etc_revdep_rebuild) + "':");

    const auto is_garbage_file = [](const FSPath & file) {
        std::string basename(file.basename());
        return '#' == basename[0] || '~' == basename[basename.length() - 1];
    };

    if (etc_revdep_rebuild_stat.is_directory_or_symlink_to_directory())
    {
        std::vector<FSPath> conf_files = std::vector<FSPath>(FSIterator(etc_revdep_rebuild, { }), FSIterator());
        conf_files.erase(
                std::remove_if(conf_files.begin(), conf_files.end(), is_garbage_file),
                conf_files.end());
        std::sort(conf_files.begin(), conf_files.end(), FSPathComparator());

        KeyValueConfigFileOptions opts;
        opts += kvcfo_disallow_space_around_equals;
        opts += kvcfo_disallow_space_inside_unquoted_values;

        for (const auto & file : conf_files)
        {
            Context ctx_file("When reading '" + stringify(file) + "':");

            if (file.stat().is_regular_file_or_symlink_to_regular_file())
            {
                KeyValueConfigFile kvs(file, opts, &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

                std::function<std::string(const std::string &)> fromfile(std::bind(&KeyValueConfigFile::get, std::cref(kvs), _1));

                from_string(fromfile, "LD_LIBRARY_MASK",  ld_library_mask);
                from_string(fromfile, "SEARCH_DIRS",      search_dirs);
                from_string(fromfile, "SEARCH_DIRS_MASK", search_dirs_mask);
            }
            else
            {
                Log::get_instance()->message("broken_linkage_finder.failure", ll_warning, lc_context)
                    << "'" << file << "' is not a regular file";
            }
        }
    }
    else if (etc_revdep_rebuild_stat.exists())
    {
        Log::get_instance()->message("broken_linkage_finder.etc_revdep_rebuild.not_a_directory", ll_warning, lc_context)
            << "'" << etc_revdep_rebuild << "' exists but is not a directory";
    }
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_profile_env(const FSPath & root)
{
    using namespace std::placeholders;

    FSPath etc_profile_env(root / "etc" / "profile.env");
    FSStat etc_profile_env_stat(etc_profile_env);
    Context ctx("When reading '" + stringify(etc_profile_env) + "':");

    if (etc_profile_env_stat.is_regular_file_or_symlink_to_regular_file())
    {
        KeyValueConfigFileOptions opts;
        opts += kvcfo_disallow_space_around_equals;
        opts += kvcfo_disallow_space_inside_unquoted_values;
        opts += kvcfo_ignore_export;

        KeyValueConfigFile kvs(etc_profile_env, opts,
                &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

        std::function<std::string (const std::string &)> fromfile(
            std::bind(&KeyValueConfigFile::get, std::cref(kvs), _1));

        from_colon_string(fromfile, "PATH",     search_dirs);
        from_colon_string(fromfile, "ROOTPATH", search_dirs);
    }
    else if (etc_profile_env_stat.exists())
        Log::get_instance()->message("broken_linkage_finder.etc_profile_env.not_a_file", ll_warning, lc_context)
            << "'" << etc_profile_env << "' exists but is not a regular file";
}

void
Imp<BrokenLinkageConfiguration>::load_from_etc_ld_so_conf(const FSPath & root)
{
    FSPath etc_ld_so_conf(root / "etc" / "ld.so.conf");
    Context ctx("When reading '" + stringify(etc_ld_so_conf) + "':");

    LineConfigFileOptions opts;
    opts += lcfo_disallow_continuations;
    opts += lcfo_allow_inline_comments;

    std::vector<std::string> res;
    parse_ld_so_conf(root, etc_ld_so_conf, opts, res);

    if (res.begin() != res.end())
    {
        Log::get_instance()->message("broken_linkage_finder.got", ll_debug, lc_context)
                << "Got " << join(res.begin(), res.end(), " ");
        std::copy(res.begin(), res.end(), create_inserter<FSPath>(std::back_inserter(search_dirs)));
        std::copy(res.begin(), res.end(), create_inserter<FSPath>(std::back_inserter(ld_so_conf)));
    }
}

void
Imp<BrokenLinkageConfiguration>::add_defaults()
{
    Context ctx("When adding default settings:");

    static const char * default_ld_library_mask[] = {
        "libodbcinst.so", "libodbc.so", "libjava.so", "libjvm.so",
    };
    static const char * default_search_dirs[] = {
        "/bin", "/sbin", "/usr/bin", "/usr/sbin", "/lib*", "/usr/lib*",
    };
    static const char * default_search_dirs_mask[] = {
        "/lib*/modules",
    };
    static const char * default_ld_so_conf[] = {
        "/lib", "/usr/lib",
    };

    Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
        << "Got LD_LIBRARY_MASK=\"" << join(std::begin(default_ld_library_mask), std::end(default_ld_library_mask), " ") << "\"";
    std::copy(std::begin(default_ld_library_mask), std::end(default_ld_library_mask),
              std::back_inserter(ld_library_mask));

    Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
        << "Got SEARCH_DIRS=\"" << join(std::begin(default_search_dirs), std::end(default_search_dirs), " ") << "\"";
    std::copy(std::begin(default_search_dirs), std::end(default_search_dirs),
              create_inserter<FSPath>(std::back_inserter(search_dirs)));

    Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
        << "Got SEARCH_DIRS_MASK=\"" << join(std::begin(default_search_dirs_mask), std::end(default_search_dirs_mask), " ") << "\"";
    std::copy(std::begin(default_search_dirs_mask), std::end(default_search_dirs_mask),
              create_inserter<FSPath>(std::back_inserter(search_dirs_mask)));

    Log::get_instance()->message("broken_linkage_finder.config", ll_debug, lc_context)
        << "Default ld.so.conf contents is \"" << join(std::begin(default_ld_so_conf), std::end(default_ld_so_conf), " ") << "\"";
    std::copy(std::begin(default_ld_so_conf), std::end(default_ld_so_conf),
              create_inserter<FSPath>(std::back_inserter(ld_so_conf)));
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::begin_search_dirs() const
{
    return DirsIterator(_imp->search_dirs.begin());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::end_search_dirs() const
{
    return DirsIterator(_imp->search_dirs.end());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::begin_ld_so_conf() const
{
    return DirsIterator(_imp->ld_so_conf.begin());
}

BrokenLinkageConfiguration::DirsIterator
BrokenLinkageConfiguration::end_ld_so_conf() const
{
    return DirsIterator(_imp->ld_so_conf.end());
}

bool
BrokenLinkageConfiguration::dir_is_masked(const FSPath & dir) const
{
    return std::binary_search(_imp->search_dirs_mask.begin(), _imp->search_dirs_mask.end(), dir, FSPathComparator());
}

bool
BrokenLinkageConfiguration::lib_is_masked(const std::string & lib) const
{
    return std::binary_search(_imp->ld_library_mask.begin(), _imp->ld_library_mask.end(), lib);
}

namespace paludis
{
    template class WrappedForwardIterator<BrokenLinkageConfiguration::DirsIteratorTag, const FSPath>;
}
