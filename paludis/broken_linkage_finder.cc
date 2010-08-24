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

#include <paludis/broken_linkage_finder.hh>
#include <paludis/broken_linkage_configuration.hh>
#include <paludis/elf_linkage_checker.hh>
#include <paludis/libtool_linkage_checker.hh>
#include <paludis/linkage_checker.hh>

#include <paludis/util/realpath.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_error.hh>

#include <paludis/contents.hh>
#include <paludis/environment.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/notifier_callback.hh>

#include <functional>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <vector>

using namespace paludis;

typedef std::multimap<FSPath, std::shared_ptr<const PackageID>, FSPathComparator> Files;
typedef std::map<FSPath, std::set<std::string>, FSPathComparator> PackageBreakage;
typedef std::map<std::shared_ptr<const PackageID>, PackageBreakage, PackageIDSetComparator> Breakage;

namespace paludis
{
    template <>
    struct Imp<BrokenLinkageFinder>
    {
        const Environment * env;
        const BrokenLinkageConfiguration config;
        std::shared_ptr<const Sequence<std::string>> libraries;

        std::vector<std::shared_ptr<LinkageChecker> > checkers;
        std::set<FSPath, FSPathComparator> extra_lib_dirs;

        Mutex mutex;

        bool has_files;
        Files files;

        Breakage breakage;
        PackageBreakage orphan_breakage;

        void search_directory(const FSPath &);

        void walk_directory(const FSPath &);
        void check_file(const FSPath &);

        void add_breakage(const FSPath &, const std::string &);
        void gather_package(const std::shared_ptr<const PackageID> &);

        Imp(const Environment * the_env, const std::shared_ptr<const Sequence<std::string>> & the_libraries) :
            env(the_env),
            config(the_env->preferred_root_key()->value()),
            libraries(the_libraries),
            has_files(false)
        {
        }
    };

    template <>
    struct WrappedForwardIteratorTraits<BrokenLinkageFinder::BrokenPackageConstIteratorTag>
    {
        typedef FirstIteratorTypes<Breakage::const_iterator>::Type UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<BrokenLinkageFinder::BrokenFileConstIteratorTag>
    {
        typedef FirstIteratorTypes<PackageBreakage::const_iterator>::Type UnderlyingIterator;
    };

    template <>
    struct WrappedForwardIteratorTraits<BrokenLinkageFinder::MissingRequirementConstIteratorTag>
    {
        typedef std::set<std::string>::const_iterator UnderlyingIterator;
    };
}

namespace
{
    const std::map<FSPath, std::set<std::string>, FSPathComparator> no_files;
    const std::set<std::string> no_reqs;

    struct ParentOf : std::unary_function<FSPath, bool>
    {
        const FSPath & _child;

        ParentOf(const FSPath & child) :
            _child(child)
        {
        }

        bool operator() (const FSPath & parent)
        {
            std::string child_str(stringify(_child)), parent_str(stringify(parent));
            return 0 == child_str.compare(0, parent_str.length(), parent_str) &&
                (parent_str.length() == child_str.length() || '/' == child_str[parent_str.length()]);
        }
    };
}

BrokenLinkageFinder::BrokenLinkageFinder(const Environment * env, const std::shared_ptr<const Sequence<std::string>> & libraries) :
    Pimp<BrokenLinkageFinder>(env, libraries)
{
    using namespace std::placeholders;

    Context ctx("When checking for broken linkage in '" + stringify(env->preferred_root_key()->value()) + "':");

    _imp->checkers.push_back(std::shared_ptr<LinkageChecker>(std::make_shared<ElfLinkageChecker>(env->preferred_root_key()->value(), libraries)));
    if (libraries->empty())
        _imp->checkers.push_back(std::shared_ptr<LinkageChecker>(std::make_shared<LibtoolLinkageChecker>(env->preferred_root_key()->value())));

    std::vector<FSPath> search_dirs_nosyms, search_dirs_pruned;
    std::transform(_imp->config.begin_search_dirs(), _imp->config.end_search_dirs(),
                   std::back_inserter(search_dirs_nosyms),
                   std::bind(realpath_with_current_and_root, _1, FSPath("/"), env->preferred_root_key()->value()));
    std::sort(search_dirs_nosyms.begin(), search_dirs_nosyms.end(), FSPathComparator());

    for (std::vector<FSPath>::const_iterator it(search_dirs_nosyms.begin()),
             it_end(search_dirs_nosyms.end()); it_end != it; ++it)
        if (search_dirs_pruned.end() ==
            std::find_if(search_dirs_pruned.begin(), search_dirs_pruned.end(),
                         ParentOf(*it)))
            search_dirs_pruned.push_back(*it);
    Log::get_instance()->message("reconcilio.broken_linkage_finder.config",
            ll_debug, lc_context) << "After resolving symlinks and pruning subdirectories, SEARCH_DIRS=\"" <<
        join(search_dirs_pruned.begin(), search_dirs_pruned.end(), " ") << "\"";

    std::transform(_imp->config.begin_ld_so_conf(), _imp->config.end_ld_so_conf(),
                   std::inserter(_imp->extra_lib_dirs, _imp->extra_lib_dirs.begin()),
                   std::bind(realpath_with_current_and_root, _1, FSPath("/"), env->preferred_root_key()->value()));

    std::for_each(search_dirs_pruned.begin(), search_dirs_pruned.end(),
                      std::bind(&Imp<BrokenLinkageFinder>::search_directory, _imp.get(), _1));

    for (std::set<FSPath>::const_iterator it(_imp->extra_lib_dirs.begin()),
             it_end(_imp->extra_lib_dirs.end()); it_end != it; ++it)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.config", ll_debug, lc_context)
            << "Need to check for extra libraries in '" << (env->preferred_root_key()->value() / *it) << "'";
        std::for_each(indirect_iterator(_imp->checkers.begin()), indirect_iterator(_imp->checkers.end()),
                std::bind(&LinkageChecker::add_extra_lib_dir, _1, env->preferred_root_key()->value() / *it));
    }

    std::function<void (const FSPath &, const std::string &)> callback(
        std::bind(&Imp<BrokenLinkageFinder>::add_breakage, _imp.get(), _1, _2));
    std::for_each(indirect_iterator(_imp->checkers.begin()), indirect_iterator(_imp->checkers.end()),
            std::bind(&LinkageChecker::need_breakage_added, _1, callback));

    _imp->checkers.clear();
}

BrokenLinkageFinder::~BrokenLinkageFinder()
{
}

void
Imp<BrokenLinkageFinder>::search_directory(const FSPath & directory)
{
    FSPath dir(directory);
    env->trigger_notifier_callback(NotifierCallbackLinkageStepEvent(dir));

    do
    {
        dir = dir.dirname();
        if (config.dir_is_masked(dir))
        {
            Log::get_instance()->message("reconcilio.broken_linkage_finder.skipping", ll_debug, lc_context)
                << "Skipping '" << directory << "' because '" << dir << "' is search-masked";
            return;
        }
    }
    while (FSPath("/") != dir);

    FSPath with_root(env->preferred_root_key()->value() / directory);
    if (with_root.stat().is_directory())
        walk_directory(with_root);
    else
        Log::get_instance()->message("reconcilio.broken_linkage_finder.missing", ll_debug, lc_context)
            << "'" << directory << "' is missing or not a directory";
}

void
Imp<BrokenLinkageFinder>::walk_directory(const FSPath & directory)
{
    using namespace std::placeholders;

    FSPath without_root(directory.strip_leading(env->preferred_root_key()->value()));
    if (config.dir_is_masked(without_root))
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.masked", ll_debug, lc_context)
            << "'" << directory << "' is search-masked";
        return;
    }

    Log::get_instance()->message("reconcilio.broken_linkage_finder.entering", ll_debug, lc_context)
        << "Entering directory '" << directory << "'";
    {
        Lock l(mutex);
        extra_lib_dirs.erase(without_root);
    }

    try
    {
        std::for_each(FSIterator(directory, { fsio_include_dotfiles, fsio_inode_sort }), FSIterator(),
                std::bind(&Imp<BrokenLinkageFinder>::check_file, this, _1));
    }
    catch (const FSError & ex)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_no_context) << ex.message();
    }
}

void
Imp<BrokenLinkageFinder>::check_file(const FSPath & file)
{
    using namespace std::placeholders;

    try
    {
        FSStat file_stat(file);

        if (file_stat.is_symlink())
        {
            FSPath target(dereference_with_root(file, env->preferred_root_key()->value()));
            if (target.stat().is_regular_file())
            {
                std::for_each(indirect_iterator(checkers.begin()), indirect_iterator(checkers.end()),
                        std::bind(&LinkageChecker::note_symlink, _1, file, target));
            }
        }

        else if (file_stat.is_directory())
            walk_directory(file);

        else if (file_stat.is_regular_file())
        {
            env->trigger_notifier_callback(NotifierCallbackLinkageStepEvent(file));

            if (indirect_iterator(checkers.end()) ==
                    std::find_if(indirect_iterator(checkers.begin()), indirect_iterator(checkers.end()),
                        std::bind(&LinkageChecker::check_file, _1, file)))
                Log::get_instance()->message("reconcilio.broken_linkage_finder.unrecognised", ll_debug, lc_context)
                    << "'" << file << "' is not a recognised file type";
        }
    }
    catch (const FSError & ex)
    {
        Log::get_instance()->message("reconcilio.broken_linkage_finder.failure", ll_warning, lc_no_context) << ex.message();
    }
}

void
Imp<BrokenLinkageFinder>::add_breakage(const FSPath & file, const std::string & req)
{
    using namespace std::placeholders;

    if (libraries->empty() && config.lib_is_masked(req))
        return;

    if (! has_files)
    {
        has_files = true;

        Context ctx("When building map from files to packages:");

        std::shared_ptr<const PackageIDSequence> pkgs((*env)[selection::AllVersionsUnsorted(
                    generator::All() | filter::InstalledAtRoot(env->preferred_root_key()->value()))]);

        std::for_each(pkgs->begin(), pkgs->end(),
                std::bind(&Imp<BrokenLinkageFinder>::gather_package, this, _1));
    }

    FSPath without_root(file.strip_leading(env->preferred_root_key()->value()));
    std::pair<Files::const_iterator, Files::const_iterator> range(files.equal_range(without_root));
    if (range.first == range.second)
        orphan_breakage[without_root].insert(req);
    else
        while (range.first != range.second)
        {
            breakage[range.first->second][without_root].insert(req);
            ++range.first;
        }
}

void
Imp<BrokenLinkageFinder>::gather_package(const std::shared_ptr<const PackageID> & pkg)
{
    using namespace std::placeholders;

    Context ctx("When gathering the contents of " + stringify(*pkg) + ":");

    std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Contents> > > key(pkg->contents_key());
    if (! key)
        return;
    std::shared_ptr<const Contents> contents(key->value());
    if (! contents)
        return;

    for (Contents::ConstIterator it(contents->begin()),
             it_end(contents->end()); it_end != it; ++it)
    {
        const ContentsFileEntry * file(simple_visitor_cast<const ContentsFileEntry>(**it));
        if (0 != file)
        {
            Lock l(mutex);
            files.insert(std::make_pair(stringify(file->location_key()->value()), pkg));
        }
    }

    pkg->can_drop_in_memory_cache();
}

BrokenLinkageFinder::BrokenPackageConstIterator
BrokenLinkageFinder::begin_broken_packages() const
{
    return BrokenPackageConstIterator(first_iterator(_imp->breakage.begin()));
}

BrokenLinkageFinder::BrokenPackageConstIterator
BrokenLinkageFinder::end_broken_packages() const
{
    return BrokenPackageConstIterator(first_iterator(_imp->breakage.end()));
}

BrokenLinkageFinder::BrokenFileConstIterator
BrokenLinkageFinder::begin_broken_files(const std::shared_ptr<const PackageID> & pkg) const
{
    if (pkg)
    {
        Breakage::const_iterator it(_imp->breakage.find(pkg));
        if (_imp->breakage.end() == it)
            return BrokenFileConstIterator(no_files.begin());

        return BrokenFileConstIterator(first_iterator(it->second.begin()));
    }
    else
        return BrokenFileConstIterator(first_iterator(_imp->orphan_breakage.begin()));
}

BrokenLinkageFinder::BrokenFileConstIterator
BrokenLinkageFinder::end_broken_files(const std::shared_ptr<const PackageID> & pkg) const
{
    if (pkg)
    {
        Breakage::const_iterator it(_imp->breakage.find(pkg));
        if (_imp->breakage.end() == it)
            return BrokenFileConstIterator(no_files.end());

        return BrokenFileConstIterator(first_iterator(it->second.end()));
    }
    else
        return BrokenFileConstIterator(first_iterator(_imp->orphan_breakage.end()));
}

BrokenLinkageFinder::MissingRequirementConstIterator
BrokenLinkageFinder::begin_missing_requirements(
    const std::shared_ptr<const PackageID> & pkg, const FSPath & file) const
{
    if (pkg)
    {
        Breakage::const_iterator pkg_it(_imp->breakage.find(pkg));
        if (_imp->breakage.end() == pkg_it)
            return MissingRequirementConstIterator(no_reqs.begin());

        PackageBreakage::const_iterator file_it(pkg_it->second.find(file));
        if (pkg_it->second.end() == file_it)
            return MissingRequirementConstIterator(no_reqs.begin());

        return MissingRequirementConstIterator(file_it->second.begin());
    }
    else
    {
        PackageBreakage::const_iterator file_it(_imp->orphan_breakage.find(file));
        if (_imp->orphan_breakage.end() == file_it)
            return MissingRequirementConstIterator(no_reqs.begin());

        return MissingRequirementConstIterator(file_it->second.begin());
    }
}

BrokenLinkageFinder::MissingRequirementConstIterator
BrokenLinkageFinder::end_missing_requirements(
    const std::shared_ptr<const PackageID> & pkg, const FSPath & file) const
{
    if (pkg)
    {
        Breakage::const_iterator pkg_it(_imp->breakage.find(pkg));
        if (_imp->breakage.end() == pkg_it)
            return MissingRequirementConstIterator(no_reqs.end());

        PackageBreakage::const_iterator file_it(pkg_it->second.find(file));
        if (pkg_it->second.end() == file_it)
            return MissingRequirementConstIterator(no_reqs.end());

        return MissingRequirementConstIterator(file_it->second.end());
    }
    else
    {
        PackageBreakage::const_iterator file_it(_imp->orphan_breakage.find(file));
        if (_imp->orphan_breakage.end() == file_it)
            return MissingRequirementConstIterator(no_reqs.end());

        return MissingRequirementConstIterator(file_it->second.end());
    }
}

template class WrappedForwardIterator<BrokenLinkageFinder::BrokenPackageConstIteratorTag,
         const std::shared_ptr<const PackageID> >;
template class WrappedForwardIterator<BrokenLinkageFinder::BrokenFileConstIteratorTag,
         const FSPath>;
template class WrappedForwardIterator<BrokenLinkageFinder::MissingRequirementConstIteratorTag, const std::string>;

