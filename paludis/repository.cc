/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/compare.hh>
#include <map>
#include <list>
#include <ctype.h>

/** \file
 * Implementation of Repository.
 *
 * \ingroup grprepository
 */

using namespace paludis;

#include <paludis/repository-sr.cc>

Repository::Repository(
        const RepositoryName & our_name,
        const RepositoryCapabilities & caps) :
    RepositoryCapabilities(caps),
    _name(our_name),
    _info(new RepositoryInfo)
{
}

Repository::~Repository()
{
}

const RepositoryName &
Repository::name() const
{
    return _name;
}

NoSuchRepositoryTypeError::NoSuchRepositoryTypeError(const std::string & format) throw ():
    ConfigurationError("No available maker for repository type '" + format + "'")
{
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

namespace paludis
{
    /**
     * Implementation data for RepositoryInfoSection.
     *
     * \ingroup grprepository
     */
    template<>
    struct Implementation<RepositoryInfoSection> :
        InternalCounted<Implementation<RepositoryInfoSection> >
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
    struct Implementation<RepositoryInfo> :
        InternalCounted<Implementation<RepositoryInfo> >
    {
        std::list<RepositoryInfoSection::ConstPointer> sections;
    };
}

RepositoryInfo &
RepositoryInfo::add_section(RepositoryInfoSection::ConstPointer s)
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

RepositoryInfo::ConstPointer
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

UseFlagName
RepositoryUseInterface::expand_flag_name(const UseFlagName & u) const
{
    std::string upper_u;
    std::transform(u.data().begin(), u.data().end(), std::back_inserter(upper_u),
                &::toupper);
    return UseFlagName(upper_u.substr(0, do_expand_flag_delim_pos(u)));
}

