/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repository.hh>
#include <paludis/repository_info.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/options.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/set-impl.hh>
#include <paludis/util/config_file.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <map>
#include <list>
#include <utility>
#include <algorithm>
#include <ctype.h>

using namespace paludis;

#include <paludis/repository-sr.cc>

template class Set<tr1::shared_ptr<Repository> >;
template class Sequence<RepositoryVirtualsEntry>;
template class Sequence<RepositoryProvidesEntry>;

namespace
{
    struct RepositoryBlacklist :
        InstantiationPolicy<RepositoryBlacklist, instantiation_method::SingletonTag>
    {
        std::map<std::string, std::string> items;

        RepositoryBlacklist()
        {
            if (! (FSEntry(DATADIR) / "paludis" / "repository_blacklist.txt").exists())
                return;

            LineConfigFile f(FSEntry(DATADIR) / "paludis" / "repository_blacklist.txt", LineConfigFileOptions());
            for (LineConfigFile::ConstIterator line(f.begin()), line_end(f.end()) ;
                    line != line_end ; ++line)
            {
                std::string::size_type p(line->find(" - "));
                if (std::string::npos != p)
                    items.insert(std::make_pair(line->substr(0, p), line->substr(p + 3)));
            }
        }
    };
}

Repository::Repository(
        const RepositoryName & our_name,
        const RepositoryCapabilities & caps,
        const std::string & f) :
    RepositoryCapabilities(caps),
    _name(our_name),
    _format(f),
    _info(new RepositoryInfo)
{
    std::map<std::string, std::string>::const_iterator i(
            RepositoryBlacklist::get_instance()->items.find(stringify(_name)));
    if (RepositoryBlacklist::get_instance()->items.end() != i)
        Log::get_instance()->message(ll_warning, lc_no_context, "Repository '" + stringify(_name) +
                "' is blacklisted with reason '" + i->second + "'. Consult the FAQ for more details.");
}

Repository::~Repository()
{
}

const RepositoryName &
Repository::name() const
{
    return _name;
}

tr1::shared_ptr<const RepositoryInfo>
Repository::info(bool) const
{
    return _info;
}

tr1::shared_ptr<const CategoryNamePartSet>
Repository::do_category_names_containing_package(const PackageNamePart & p) const
{
    Context context("When finding category names containing package '" + stringify(p) + "':");

    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    tr1::shared_ptr<const CategoryNamePartSet> cats(category_names());
    for (CategoryNamePartSet::ConstIterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
        if (has_package_named(*c + p))
            result->insert(*c);

    return result;
}

void
Repository::regenerate_cache() const
{
}

std::string
Repository::format() const
{
    return _format;
}

RepositoryInstalledInterface::~RepositoryInstalledInterface()
{
}

RepositorySetsInterface::~RepositorySetsInterface()
{
}

RepositorySyncableInterface::~RepositorySyncableInterface()
{
}

RepositoryUseInterface::~RepositoryUseInterface()
{
}

RepositoryWorldInterface::~RepositoryWorldInterface()
{
}

RepositoryEnvironmentVariableInterface::~RepositoryEnvironmentVariableInterface()
{
}

RepositoryMirrorsInterface::~RepositoryMirrorsInterface()
{
}

RepositoryProvidesInterface::~RepositoryProvidesInterface()
{
}

RepositoryVirtualsInterface::~RepositoryVirtualsInterface()
{
}

RepositoryDestinationInterface::~RepositoryDestinationInterface()
{
}

RepositoryLicensesInterface::~RepositoryLicensesInterface()
{
}

RepositoryEInterface::~RepositoryEInterface()
{
}

RepositoryHookInterface::~RepositoryHookInterface()
{
}

RepositoryMakeVirtualsInterface::~RepositoryMakeVirtualsInterface()
{
}

RepositoryQAInterface::~RepositoryQAInterface()
{
}

RepositoryManifestInterface::~RepositoryManifestInterface()
{
}

tr1::shared_ptr<FSEntry>
RepositoryLicensesInterface::license_exists(const std::string & license) const
{
    return do_license_exists(license);
}

UseFlagState
RepositoryUseInterface::query_use(const UseFlagName & u, const PackageID & pde) const
{
    if (do_query_use_mask(u, pde))
        return use_disabled;
    else if (do_query_use_force(u, pde))
        return use_enabled;
    else
        return do_query_use(u, pde);
}

bool
RepositoryUseInterface::query_use_mask(const UseFlagName & u, const PackageID & pde) const
{
    return do_query_use_mask(u, pde);
}

bool
RepositoryUseInterface::query_use_force(const UseFlagName & u, const PackageID & pde) const
{
    return do_query_use_force(u, pde);
}

tr1::shared_ptr<const UseFlagNameSet>
RepositoryUseInterface::arch_flags() const
{
    return do_arch_flags();
}

tr1::shared_ptr<const UseFlagNameSet>
RepositoryUseInterface::use_expand_flags() const
{
    return do_use_expand_flags();
}

tr1::shared_ptr<const UseFlagNameSet>
RepositoryUseInterface::use_expand_hidden_prefixes() const
{
    return do_use_expand_hidden_prefixes();
}

tr1::shared_ptr<const UseFlagNameSet>
RepositoryUseInterface::use_expand_prefixes() const
{
    return do_use_expand_prefixes();
}

std::string
RepositoryUseInterface::describe_use_flag(const UseFlagName & n, const PackageID & pkg) const
{
    return do_describe_use_flag(n, pkg);
}

bool
RepositoryMirrorsInterface::is_mirror(const std::string & s) const
{
    return begin_mirrors(s) != end_mirrors(s);
}

tr1::shared_ptr<const PackageIDSequence>
Repository::package_ids(const QualifiedPackageName & p) const
{
    return do_package_ids(p);
}

bool
Repository::can_be_favourite_repository() const
{
    return true;
}

bool
Repository::has_package_named(const QualifiedPackageName & q) const
{
    return do_has_package_named(q);
}

bool
Repository::has_category_named(const CategoryNamePart & q) const
{
    return do_has_category_named(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
Repository::category_names_containing_package(const PackageNamePart & p) const
{
    return do_category_names_containing_package(p);
}

tr1::shared_ptr<const QualifiedPackageNameSet>
Repository::package_names(const CategoryNamePart & c) const
{
    return do_package_names(c);
}

tr1::shared_ptr<const CategoryNamePartSet>
Repository::category_names() const
{
    return do_category_names();
}

bool
RepositorySyncableInterface::sync() const
{
    return do_sync();
}

bool
Repository::some_ids_might_support_action(const SupportsActionTestBase & b) const
{
    return do_some_ids_might_support_action(b);
}

