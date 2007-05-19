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

#include "environment_implementation.hh"
#include <paludis/package_database_entry.hh>
#include <paludis/version_metadata.hh>
#include <paludis/package_database.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/eapi.hh>
#include <algorithm>

using namespace paludis;

namespace
{
    struct LicenceChecker :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<LicenceChecker, AllDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<LicenceChecker, AllDepSpec>::visit;

        bool ok;
        const EnvironmentImplementation * const env;
        bool (EnvironmentImplementation::* const func) (const std::string &, const PackageDatabaseEntry &) const;
        const PackageDatabaseEntry * const db_entry;

        LicenceChecker(const EnvironmentImplementation * const e,
                bool (EnvironmentImplementation::* const f) (const std::string &, const PackageDatabaseEntry &) const,
                const PackageDatabaseEntry * const d) :
            ok(true),
            env(e),
            func(f),
            db_entry(d)
        {
        }

        void visit(const AnyDepSpec * spec)
        {
            bool local_ok(false);

            if (spec->begin() == spec->end())
                local_ok = true;
            else
            {
                for (CompositeDepSpec::Iterator i(spec->begin()), i_end(spec->end()) ;
                        i != i_end ; ++i)
                {
                    Save<bool> save_ok(&ok, true);
                    (*i)->accept(this);
                    local_ok |= ok;
                }
            }

            ok &= local_ok;
        }

        void visit(const UseDepSpec * spec)
        {
            if (env->query_use(spec->flag(), *db_entry))
                std::for_each(spec->begin(), spec->end(), accept_visitor(this));
        }

        void visit(const PlainTextDepSpec * spec)
        {
            if (! (env->*func)(spec->text(), *db_entry))
                ok = false;
        }

        void visit(const PackageDepSpec *) PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Encountered PackageDepSpec in licence?");
        }

        void visit(const BlockDepSpec *)  PALUDIS_ATTRIBUTE((noreturn))
        {
            throw InternalError(PALUDIS_HERE, "Encountered BlockDepSpec in licence?");
        }
    };
}

bool
EnvironmentImplementation::accept_eapi(const EAPI & e, const PackageDatabaseEntry &) const
{
    return e.supported;
}

bool
EnvironmentImplementation::accept_keywords(std::tr1::shared_ptr<const KeywordNameCollection> k,
        const PackageDatabaseEntry &) const
{
    return k->end() != k->find(KeywordName("*"));
}

bool
EnvironmentImplementation::accept_license(const std::string &, const PackageDatabaseEntry &) const
{
    return true;
}

bool
EnvironmentImplementation::accept_breaks_portage(const PackageDatabaseEntry &) const
{
    return true;
}

bool
EnvironmentImplementation::accept_interactive(const PackageDatabaseEntry &) const
{
    return false;
}

bool
EnvironmentImplementation::masked_by_user(const PackageDatabaseEntry &) const
{
    return false;
}

bool
EnvironmentImplementation::unmasked_by_user(const PackageDatabaseEntry &) const
{
    return false;
}

bool
EnvironmentImplementation::breaks_portage(const PackageDatabaseEntry & e, const VersionMetadata & m) const
{
    return (e.version.has_try_part() || e.version.has_scm_part()
            || (! m.eapi.supported) || (m.eapi.supported->breaks_portage));
}

EnvironmentImplementation::~EnvironmentImplementation()
{
}


std::tr1::shared_ptr<const UseFlagNameCollection>
EnvironmentImplementation::known_use_expand_names(const UseFlagName &, const PackageDatabaseEntry &) const
{
    static std::tr1::shared_ptr<const UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
EnvironmentImplementation::bashrc_files() const
{
    static std::tr1::shared_ptr<const FSEntryCollection> result(new FSEntryCollection::Concrete);
    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
EnvironmentImplementation::syncers_dirs() const
{
    std::tr1::shared_ptr<FSEntryCollection> result(new FSEntryCollection::Concrete);
    result->push_back(FSEntry(DATADIR "/paludis/syncers"));
    result->push_back(FSEntry(LIBEXECDIR "/paludis/syncers"));
    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
EnvironmentImplementation::fetchers_dirs() const
{
    std::tr1::shared_ptr<FSEntryCollection> result(new FSEntryCollection::Concrete);
    result->push_back(FSEntry(DATADIR "/paludis/fetchers"));
    result->push_back(FSEntry(LIBEXECDIR "/paludis/fetchers"));
    return result;
}

std::tr1::shared_ptr<const FSEntryCollection>
EnvironmentImplementation::hook_dirs() const
{
    static std::tr1::shared_ptr<const FSEntryCollection> result(new FSEntryCollection::Concrete);
    return result;
}

const FSEntry
EnvironmentImplementation::root() const
{
    return FSEntry("/");
}

uid_t
EnvironmentImplementation::reduced_uid() const
{
    return getuid();
}

gid_t
EnvironmentImplementation::reduced_gid() const
{
    return getgid();
}

std::tr1::shared_ptr<const MirrorsCollection>
EnvironmentImplementation::mirrors(const std::string &) const
{
    static std::tr1::shared_ptr<const MirrorsCollection> result(new MirrorsCollection::Concrete);
    return result;
}

std::tr1::shared_ptr<const SetNameCollection>
EnvironmentImplementation::set_names() const
{
    static std::tr1::shared_ptr<const SetNameCollection> result(new SetNameCollection::Concrete);
    return result;
}

HookResult
EnvironmentImplementation::perform_hook(const Hook &) const
{
    return HookResult(0, "");
}

std::tr1::shared_ptr<const DestinationsCollection>
EnvironmentImplementation::default_destinations() const
{
    std::tr1::shared_ptr<DestinationsCollection> result(new DestinationsCollection::Concrete);

    for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
        if ((*r)->destination_interface)
            if ((*r)->destination_interface->is_default_destination())
                result->insert(*r);

    return result;
}

MaskReasons
EnvironmentImplementation::mask_reasons(const PackageDatabaseEntry & e, const MaskReasonsOptions & options) const
{
    Context context("When checking for mask reasons for '" + stringify(e) + "':");

    MaskReasons result;
    std::tr1::shared_ptr<const VersionMetadata> metadata(package_database()->fetch_repository(
                e.repository)->version_metadata(e.name, e.version));

    if (! accept_eapi(metadata->eapi, e))
    {
        result += mr_eapi;
        return result;
    }

    if (breaks_portage(e, *metadata) && ! accept_breaks_portage(e))
        result += mr_breaks_portage;

    if (metadata->interactive && ! accept_interactive(e))
        result += mr_interactive;

    if (metadata->virtual_interface)
    {
        result |= mask_reasons(metadata->virtual_interface->virtual_for);
        if (result.any())
            result += mr_by_association;
    }

    if (metadata->ebuild_interface)
    {
        std::tr1::shared_ptr<const KeywordNameCollection> keywords(metadata->ebuild_interface->keywords());
        if (! accept_keywords(keywords, e))
        {
            do
            {
                if (options[mro_override_unkeyworded])
                {
                    std::tr1::shared_ptr<KeywordNameCollection> minus_keywords(new KeywordNameCollection::Concrete);
                    for (KeywordNameCollection::Iterator k(keywords->begin()), k_end(keywords->end()) ;
                            k != k_end ; ++k)
                        if ('-' == stringify(*k).at(0))
                            minus_keywords->insert(KeywordName(stringify(*k).substr(1)));

                    if (! accept_keywords(minus_keywords, e))
                        continue;
                }

                if (options[mro_override_tilde_keywords])
                {
                    std::tr1::shared_ptr<KeywordNameCollection> detildeified_keywords(new KeywordNameCollection::Concrete);
                    for (KeywordNameCollection::Iterator k(keywords->begin()), k_end(keywords->end()) ;
                            k != k_end ; ++k)
                    {
                        detildeified_keywords->insert(*k);
                        if ('~' == stringify(*k).at(0))
                            detildeified_keywords->insert(KeywordName(stringify(*k).substr(1)));
                    }

                    if (accept_keywords(detildeified_keywords, e))
                        continue;
                }

                result += mr_keyword;
            } while (false);
        }
    }

    if (metadata->license_interface)
    {
        LicenceChecker lc(this, &EnvironmentImplementation::accept_license, &e);
        metadata->license_interface->license()->accept(&lc);
        if (! lc.ok)
            result += mr_license;
    }

    if (! unmasked_by_user(e))
    {
        if (masked_by_user(e))
            result += mr_user_mask;

        std::tr1::shared_ptr<const Repository> repo(package_database()->fetch_repository(e.repository));
        if (repo->mask_interface)
        {
            if (repo->mask_interface->query_profile_masks(e.name, e.version))
                result += mr_profile_mask;

            if (repo->mask_interface->query_repository_masks(e.name, e.version))
                result += mr_repository_mask;
        }
    }

    return result;
}

std::tr1::shared_ptr<DepSpec>
EnvironmentImplementation::set(const SetName & s) const
{
    std::tr1::shared_ptr<CompositeDepSpec> result(local_set(s));
    if (result)
        return result;

    /* these sets always exist, even if empty */
    if (s.data() == "everything" || s.data() == "system" || s.data() == "world" || s.data() == "security")
        result.reset(new AllDepSpec);

    for (PackageDatabase::RepositoryIterator r(package_database()->begin_repositories()),
            r_end(package_database()->end_repositories()) ;
            r != r_end ; ++r)
    {
        if (! (*r)->sets_interface)
            continue;

        std::tr1::shared_ptr<DepSpec> add((*r)->sets_interface->package_set(s));
        if (0 != add)
        {
            if (! result)
                result.reset(new AllDepSpec);
            result->add_child(add);
        }

        if ("everything" == s.data() || "world" == s.data())
        {
            add = (*r)->sets_interface->package_set(SetName("system"));
            if (0 != add)
                result->add_child(add);
        }
    }

    return result;
}

bool
EnvironmentImplementation::query_use(const UseFlagName & f, const PackageDatabaseEntry & e) const
{
    std::tr1::shared_ptr<const Repository> repo(package_database()->fetch_repository(e.repository));

    if (repo && repo->use_interface)
    {
        if (repo->use_interface->query_use_mask(f, e))
            return false;
        if (repo->use_interface->query_use_force(f, e))
            return true;
    }

    if (repo && repo->use_interface)
    {
        switch (repo->use_interface->query_use(f, e))
        {
            case use_disabled:
            case use_unspecified:
                return false;

            case use_enabled:
                return true;

            case last_use:
                ;
        }

        throw InternalError(PALUDIS_HERE, "bad state");
    }
    else
        return false;
}

std::tr1::shared_ptr<CompositeDepSpec>
EnvironmentImplementation::local_set(const SetName &) const
{
    return std::tr1::shared_ptr<CompositeDepSpec>();
}

