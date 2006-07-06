/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

/** \file
 * Implementation of Repository.
 *
 * \ingroup grprepository
 */

using namespace paludis;

Repository::Repository(
        const RepositoryName & name,
        const RepositoryCapabilities & caps) :
    _name(name),
    _caps(caps),
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

RepositoryInfo &
RepositoryInfo::add_section(const RepositoryInfoSection & s)
{
    _sections.push_back(s);
    return *this;
}

RepositoryInfoSection::RepositoryInfoSection(const std::string & heading) :
    _heading(heading)
{
}

RepositoryInfoSection &
RepositoryInfoSection::add_kv(const std::string & k, const std::string & v)
{
    _kvs.insert(std::make_pair(k, v));
    return *this;
}

RepositoryInfo::ConstPointer
Repository::info(bool) const
{
    return _info;
}

