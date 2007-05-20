/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/repositories/gentoo/portage_repository.hh>
#include <paludis/repositories/gentoo/portage_repository_sets.hh>
#include <paludis/repositories/gentoo/glsa.hh>

#include <paludis/environment.hh>
#include <paludis/config_file.hh>
#include <paludis/query.hh>
#include <paludis/set_file.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>

#include <list>
#include <paludis/util/tr1_functional.hh>
#include <set>

#include "config.h"

using namespace paludis;

namespace paludis
{
    /**
     * Implementation data for PortageRepositorySets.
     *
     * \ingroup grpportagerepository
     */
    template<>
    struct Implementation<PortageRepositorySets>
    {
        const Environment * const environment;
        const PortageRepository * const portage_repository;
        const PortageRepositoryParams params;

        Implementation(const Environment * const e, const PortageRepository * const p,
                const PortageRepositoryParams & k) :
            environment(e),
            portage_repository(p),
            params(k)
        {
        }
    };
}

PortageRepositorySets::PortageRepositorySets(const Environment * const e, const PortageRepository * const p,
        const PortageRepositoryParams & k) :
    PrivateImplementationPattern<PortageRepositorySets>(new Implementation<PortageRepositorySets>(e, p, k))
{
}

PortageRepositorySets::~PortageRepositorySets()
{
}


tr1::shared_ptr<DepSpec>
PortageRepositorySets::package_set(const SetName & s) const
{
    if ("system" == s.data())
        throw InternalError(PALUDIS_HERE, "system set should've been handled by PortageRepository");
    else if ("security" == s.data())
        return security_set(false);
    else if ("insecurity" == s.data())
        return security_set(true);
    else if ((_imp->params.setsdir / (stringify(s) + ".conf")).exists())
    {
        tr1::shared_ptr<GeneralSetDepTag> tag(new GeneralSetDepTag(s, stringify(_imp->portage_repository->name())));

        FSEntry ff(_imp->params.setsdir / (stringify(s) + ".conf"));
        Context context("When loading package set '" + stringify(s) + "' from '" + stringify(ff) + "':");

        SetFile f(SetFileParams::create()
                .file_name(ff)
                .environment(_imp->environment)
                .type(sft_paludis_conf)
                .parse_mode(pds_pm_eapi_0)
                .tag(tag));


        return f.contents();
    }
    else
        return tr1::shared_ptr<DepSpec>();
}

tr1::shared_ptr<const SetNameCollection>
PortageRepositorySets::sets_list() const
{
    Context context("While generating the list of sets:");

    tr1::shared_ptr<SetNameCollection> result(new SetNameCollection::Concrete);
    result->insert(SetName("insecurity"));
    result->insert(SetName("security"));
    result->insert(SetName("system"));

    try
    {
        using namespace tr1::placeholders;

        std::list<FSEntry> repo_sets;
        std::copy(DirIterator(_imp->params.setsdir), DirIterator(),
            filter_inserter(std::back_inserter(repo_sets), tr1::bind(is_file_with_extension, _1, ".conf", IsFileWithOptions())));

        std::list<FSEntry>::const_iterator f(repo_sets.begin()),
            f_end(repo_sets.end());

        for ( ; f != f_end ; ++f)
            try
            {
                result->insert(SetName(stringify(*f)));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping set '"
                        + stringify(*f) + "' due to exception '" + stringify(e.message()) + "' ("
                        + stringify(e.what()) + ")");
            }
    }
    catch (const paludis::DirOpenError &)
    {
    }

    return result;
}

PackageDatabaseEntryCollection::Iterator
PortageRepositorySets::find_best(PackageDatabaseEntryCollection & c, const PackageDatabaseEntry & e) const
{
    Context local("When finding best update for '" + stringify(e.name) + "-" +
            stringify(e.version) + "':");
    // Find an entry in c that matches e best. e is not in c.
    QualifiedPackageName n(e.name);
    SlotName s(_imp->environment->package_database()->fetch_repository(
                e.repository)->version_metadata(e.name, e.version)->slot);
    PackageDatabaseEntryCollection::Iterator i(c.begin()), i_end(c.end()), i_best(c.end());
    for ( ; i != i_end; ++i)
    {
        if (n != i->name)
            continue;
        if (s != _imp->environment->package_database()->fetch_repository(
                    i->repository)->version_metadata(
                    i->name, i->version)->slot)
            continue;

        i_best = i;
    }

    return i_best;
}

namespace
{
    bool
    match_range(const PackageDatabaseEntry & e, const GLSARange & r)
    {
        VersionOperatorValue our_op(static_cast<VersionOperatorValue>(-1));
        std::string ver(r.version);
        if (r.op == "le")
            our_op = vo_less_equal;
        if (r.op == "lt")
            our_op = vo_less;
        if (r.op == "eq")
        {
            if (! ver.empty() && '*' == ver.at(ver.length() - 1))
            {
                ver.erase(ver.length() - 1);
                our_op = vo_equal_star;
            }
            else
                our_op = vo_equal;
        }
        if (r.op == "gt")
            our_op = vo_greater;
        if (r.op == "ge")
            our_op = vo_greater_equal;

        if (-1 != our_op)
            return (VersionOperator(our_op).as_version_spec_comparator()(e.version, VersionSpec(ver)));

        if (0 == r.op.compare(0, 1, "r"))
        {
            return (VersionOperator(our_op).as_version_spec_comparator()(e.version, VersionSpec(ver))) &&
                match_range(e, GLSARange::create().op(r.op.substr(1)).version(r.version));
        }

        throw GLSAError("Got bad op '" + r.op + "'");
    }

    bool
    is_vulnerable(const GLSAPackage & glsa_pkg, const PackageDatabaseEntry & c)
    {
        /* a package is affected if it matches any vulnerable line, except if it matches
         * any unaffected line. */
        bool vulnerable(false);
        for (GLSAPackage::RangesIterator r(glsa_pkg.begin_vulnerable()), r_end(glsa_pkg.end_vulnerable()) ;
                r != r_end && ! vulnerable ; ++r)
            if (match_range(c, *r))
                vulnerable = true;

        if (! vulnerable)
            return false;

        for (GLSAPackage::RangesIterator r(glsa_pkg.begin_unaffected()), r_end(glsa_pkg.end_unaffected()) ;
                r != r_end && vulnerable ; ++r)
            if (match_range(c, *r))
                vulnerable = false;

        return vulnerable;
    }
}

tr1::shared_ptr<DepSpec>
PortageRepositorySets::security_set(bool insecurity) const
{
    Context context("When building security or insecurity package set:");
    tr1::shared_ptr<AllDepSpec> security_packages(new AllDepSpec);

    if (!_imp->params.securitydir.is_directory_or_symlink_to_directory())
        return security_packages;

    std::map<std::string, tr1::shared_ptr<GLSADepTag> > glsa_tags;

    for (DirIterator f(_imp->params.securitydir), f_end ; f != f_end; ++f)
    {
        if (! is_file_with_prefix_extension(*f, "glsa-", ".xml", IsFileWithOptions()))
            continue;

        Context local_context("When parsing security advisory '" + stringify(*f) + "':");

        try
        {
            tr1::shared_ptr<const GLSA> glsa(GLSA::create_from_xml_file(stringify(*f)));
            Context local_local_context("When handling GLSA '" + glsa->id() + "' from '" +
                    stringify(*f) + "':");

            for (GLSA::PackagesIterator glsa_pkg(glsa->begin_packages()),
                    glsa_pkg_end(glsa->end_packages()) ; glsa_pkg != glsa_pkg_end ; ++glsa_pkg)
            {
                tr1::shared_ptr<const PackageDatabaseEntryCollection> candidates;
                if (insecurity)
                    candidates = _imp->environment->package_database()->query(query::Package(glsa_pkg->name()), qo_order_by_version);
                else
                    candidates = _imp->environment->package_database()->query(
                            query::Package(glsa_pkg->name()) & query::RepositoryHasInstalledInterface(), qo_order_by_version);

                for (PackageDatabaseEntryCollection::Iterator c(candidates->begin()), c_end(candidates->end()) ;
                        c != c_end ; ++c)
                {
                    if (! is_vulnerable(*glsa_pkg, *c))
                        continue;

                    if (glsa_tags.end() == glsa_tags.find(glsa->id()))
                        glsa_tags.insert(std::make_pair(glsa->id(), tr1::shared_ptr<GLSADepTag>(
                                        new GLSADepTag(glsa->id(), glsa->title()))));

                    if (insecurity)
                    {
                        tr1::shared_ptr<VersionRequirements> v(new VersionRequirements::Concrete);
                        v->push_back(VersionRequirement(vo_equal, c->version));
                        tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                                    tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(c->name)),
                                    tr1::shared_ptr<CategoryNamePart>(),
                                    tr1::shared_ptr<PackageNamePart>(),
                                    v, vr_and,
                                    tr1::shared_ptr<SlotName>(),
                                    tr1::shared_ptr<RepositoryName>(new RepositoryName(c->repository))));
                        spec->set_tag(glsa_tags.find(glsa->id())->second);
                        security_packages->add_child(spec);
                    }
                    else
                    {
                        /* we need to find the best not vulnerable installable package that isn't masked
                         * that's in the same slot as our vulnerable installed package. */
                        bool ok(false);
                        SlotName wanted_slot(_imp->environment->package_database()->fetch_repository(
                                    c->repository)->version_metadata(c->name, c->version)->slot);
                        tr1::shared_ptr<const PackageDatabaseEntryCollection> available(
                                _imp->environment->package_database()->query(
                                    query::Matches(PackageDepSpec(
                                            tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(
                                                    glsa_pkg->name())),
                                            tr1::shared_ptr<CategoryNamePart>(),
                                            tr1::shared_ptr<PackageNamePart>(),
                                            tr1::shared_ptr<VersionRequirements>(),
                                            vr_and,
                                            tr1::shared_ptr<SlotName>(new SlotName(wanted_slot)))) &
                                    query::RepositoryHasInstallableInterface() &
                                    query::NotMasked(),
                                    qo_order_by_version));

                        for (PackageDatabaseEntryCollection::ReverseIterator r(available->rbegin()),
                                r_end(available->rend()) ; r != r_end ; ++r)
                        {
                            if (is_vulnerable(*glsa_pkg, *r))
                            {
                                Log::get_instance()->message(ll_debug, lc_context, "Skipping '" + stringify(*r)
                                        + "' due to is_vulnerable match");
                                continue;
                            }

                            tr1::shared_ptr<VersionRequirements> v(new VersionRequirements::Concrete);
                            v->push_back(VersionRequirement(vo_equal, r->version));
                            tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(
                                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(r->name)),
                                        tr1::shared_ptr<CategoryNamePart>(),
                                        tr1::shared_ptr<PackageNamePart>(),
                                        v, vr_and,
                                        tr1::shared_ptr<SlotName>(),
                                        tr1::shared_ptr<RepositoryName>(new RepositoryName(r->repository))));
                            spec->set_tag(glsa_tags.find(glsa->id())->second);
                            security_packages->add_child(spec);
                            ok = true;
                            break;
                        }

                        if (! ok)
                            throw GLSAError("Could not determine upgrade path to resolve '"
                                    + glsa->id() + ": " + glsa->title() + "' for package '"
                                    + stringify(*c) + "'");
                    }
                }
            }
        }
        catch (const GLSAError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Cannot use GLSA '" +
                    stringify(*f) + "' due to exception '" + e.message() + "' (" + e.what() + ")");
        }
        catch (const NameError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Cannot use GLSA '" +
                    stringify(*f) + "' due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }

    return security_packages;
}

