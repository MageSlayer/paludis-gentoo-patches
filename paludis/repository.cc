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
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/config_file.hh>
#include <map>
#include <list>
#include <algorithm>
#include <ctype.h>

/** \file
 * Implementation of Repository.
 *
 * \ingroup grprepository
 */

using namespace paludis;

#include <paludis/repository-sr.cc>

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
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
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

PackageActionError::PackageActionError(const std::string & msg) throw () :
    Exception(msg)
{
}

PackageInstallActionError::PackageInstallActionError(const std::string & msg) throw () :
    PackageActionError("Install error: " + msg)
{
}

EnvironmentVariableActionError::EnvironmentVariableActionError(const std::string & msg) throw () :
    PackageActionError("Environment variable query error: " + msg)
{
}

PackageFetchActionError::PackageFetchActionError(const std::string & msg) throw () :
    PackageActionError("Fetch error: " + msg)
{
}

PackageUninstallActionError::PackageUninstallActionError(const std::string & msg) throw () :
    PackageActionError("Uninstall error: " + msg)
{
}

PackageConfigActionError::PackageConfigActionError(const std::string & msg) throw () :
    PackageActionError("Configuration error: " + msg)
{
}

namespace paludis
{
    /**
     * Implementation data for RepositoryInfoSection.
     *
     * \ingroup grprepository
     */
    template<>
    struct Implementation<RepositoryInfoSection>
    {
        std::string heading;
        std::map<std::string, std::string> kvs;
    };

    /**
     * Implementation data for RepositoryInfo.
     *
     * \ingroup grprepository
     */
    template<>
    struct Implementation<RepositoryInfo>
    {
        std::list<std::tr1::shared_ptr<const RepositoryInfoSection> > sections;
    };
}

RepositoryInfo &
RepositoryInfo::add_section(std::tr1::shared_ptr<const RepositoryInfoSection> s)
{
    _imp->sections.push_back(s);
    return *this;
}

RepositoryInfoSection::RepositoryInfoSection(const std::string & our_heading) :
    PrivateImplementationPattern<RepositoryInfoSection>(new Implementation<RepositoryInfoSection>)
{
    _imp->heading = our_heading;
}

RepositoryInfoSection::~RepositoryInfoSection()
{
}

std::string
RepositoryInfoSection::heading() const
{
    return _imp->heading;
}

RepositoryInfoSection::KeyValueIterator
RepositoryInfoSection::begin_kvs() const
{
    return KeyValueIterator(_imp->kvs.begin());
}

RepositoryInfoSection::KeyValueIterator
RepositoryInfoSection::end_kvs() const
{
    return KeyValueIterator(_imp->kvs.end());
}

RepositoryInfoSection &
RepositoryInfoSection::add_kv(const std::string & k, const std::string & v)
{
    _imp->kvs.insert(std::make_pair(k, v));
    return *this;
}

std::string
RepositoryInfoSection::get_key_with_default(const std::string & k, const std::string & d) const
{
    std::map<std::string, std::string>::const_iterator p(_imp->kvs.find(k));
    if (p != _imp->kvs.end())
        return p->second;
    else
        return d;
}

std::tr1::shared_ptr<const RepositoryInfo>
Repository::info(bool) const
{
    return _info;
}

RepositoryInfo::RepositoryInfo() :
    PrivateImplementationPattern<RepositoryInfo>(new Implementation<RepositoryInfo>)
{
}

RepositoryInfo::~RepositoryInfo()
{
}

RepositoryInfo::SectionIterator
RepositoryInfo::begin_sections() const
{
    return SectionIterator(_imp->sections.begin());
}

RepositoryInfo::SectionIterator
RepositoryInfo::end_sections() const
{
    return SectionIterator(_imp->sections.end());
}

std::string
RepositoryInfo::get_key_with_default(const std::string & k, const std::string & d) const
{
    std::string result(d);
    for (SectionIterator i(begin_sections()), i_end(end_sections()) ; i != i_end ; ++i)
        result = (*i)->get_key_with_default(k, result);
    return result;
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
Repository::do_category_names_containing_package(const PackageNamePart & p) const
{
    Context context("When finding category names containing package '" + stringify(p) + "':");

    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    std::tr1::shared_ptr<const CategoryNamePartCollection> cats(category_names());
    for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
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

RepositoryInstallableInterface::~RepositoryInstallableInterface()
{
}

RepositoryInstalledInterface::~RepositoryInstalledInterface()
{
}

RepositoryMaskInterface::~RepositoryMaskInterface()
{
}

RepositorySetsInterface::~RepositorySetsInterface()
{
}

RepositorySyncableInterface::~RepositorySyncableInterface()
{
}

RepositoryUninstallableInterface::~RepositoryUninstallableInterface()
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

RepositoryContentsInterface::~RepositoryContentsInterface()
{
}

RepositoryConfigInterface::~RepositoryConfigInterface()
{
}

RepositoryLicensesInterface::~RepositoryLicensesInterface()
{
}

RepositoryPortageInterface::~RepositoryPortageInterface()
{
}

RepositoryHookInterface::~RepositoryHookInterface()
{
}

RepositoryPretendInterface::~RepositoryPretendInterface()
{
}

bool
RepositoryPretendInterface::pretend(const QualifiedPackageName & q, const VersionSpec & v) const
{
    return do_pretend(q, v);
}

UseFlagState
RepositoryUseInterface::query_use(const UseFlagName & u, const PackageDatabaseEntry & pde) const
{
    if (do_query_use_mask(u, pde))
        return use_disabled;
    else if (do_query_use_force(u, pde))
        return use_enabled;
    else
        return do_query_use(u, pde);
}

bool
RepositoryUseInterface::query_use_mask(const UseFlagName & u, const PackageDatabaseEntry & pde) const
{
    return do_query_use_mask(u, pde);
}

bool
RepositoryUseInterface::query_use_force(const UseFlagName & u, const PackageDatabaseEntry & pde) const
{
    return do_query_use_force(u, pde);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
RepositoryUseInterface::arch_flags() const
{
    return do_arch_flags();
}

std::tr1::shared_ptr<const UseFlagNameCollection>
RepositoryUseInterface::use_expand_flags() const
{
    return do_use_expand_flags();
}

std::tr1::shared_ptr<const UseFlagNameCollection>
RepositoryUseInterface::use_expand_hidden_prefixes() const
{
    return do_use_expand_hidden_prefixes();
}

std::tr1::shared_ptr<const UseFlagNameCollection>
RepositoryUseInterface::use_expand_prefixes() const
{
    return do_use_expand_prefixes();
}

std::string
RepositoryUseInterface::describe_use_flag(const UseFlagName & n, const PackageDatabaseEntry & pkg) const
{
    return do_describe_use_flag(n, pkg);
}

