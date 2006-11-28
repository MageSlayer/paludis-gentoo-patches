/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/repositories/portage/portage_repository_sets.hh>
#include <paludis/repositories/portage/glsa.hh>

#include <paludis/dep_list/dep_list.hh>
#include <paludis/environment.hh>
#include <paludis/config_file.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>

#include <list>
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
    struct Implementation<PortageRepositorySets> :
        InternalCounted<Implementation<PortageRepositorySets> >
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


DepAtom::Pointer
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
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag(s, stringify(_imp->portage_repository->name())));

        FSEntry ff(_imp->params.setsdir / (stringify(s) + ".conf"));
        Context context("When loading package set '" + stringify(s) + "' from '" + stringify(ff) + "':");

        AllDepAtom::Pointer result(new AllDepAtom);
        LineConfigFile f(ff);
        for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ;
                line != line_end ; ++line)
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(tokens));
            if (tokens.empty())
                continue;

            if (1 == tokens.size())
            {
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not specify '*' or '?', assuming '*'");
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(0)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("*" == tokens.at(0))
            {
                PackageDepAtom::Pointer atom(new PackageDepAtom(tokens.at(1)));
                atom->set_tag(tag);
                result->add_child(atom);
            }
            else if ("?" == tokens.at(0))
            {
                PackageDepAtom::Pointer p(new PackageDepAtom(tokens.at(1)));
                p->set_tag(tag);
                if (! _imp->environment->package_database()->query(
                            PackageDepAtom::Pointer(new PackageDepAtom(p->package())),
                            is_installed_only)->empty())
                    result->add_child(p);
            }
            else
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' does not start with '*' or '?' token, skipping");

            if (tokens.size() > 2)
                Log::get_instance()->message(ll_warning, lc_context,
                        "Line '" + *line + "' in set file '"
                        + stringify(ff) + "' has trailing garbage");
        }

        return result;
    }
    else
        return DepAtom::Pointer(0);
}

SetsCollection::ConstPointer
PortageRepositorySets::sets_list() const
{
    Context context("While generating the list of sets:");

    SetsCollection::Pointer result(new SetsCollection::Concrete);
    result->insert(SetName("insecurity"));
    result->insert(SetName("security"));
    result->insert(SetName("system"));

    try
    {
        std::list<FSEntry> repo_sets;
        std::copy(DirIterator(_imp->params.setsdir), DirIterator(),
            filter_inserter(std::back_inserter(repo_sets),
            IsFileWithExtension(".conf")));

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
    catch (const paludis::DirOpenError & e)
    {
    }

    return result;
}

namespace
{
    inline
    PackageDepAtom::Pointer make_atom(const PackageDatabaseEntry & e)
    {
        QualifiedPackageName n(e.name);
        VersionSpec v(e.version);

        std::string s("=" + stringify(n) + "-" + stringify(v));
        return PackageDepAtom::Pointer(new PackageDepAtom(s));
    }
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
            return (e.version.*(VersionOperator(our_op).as_version_spec_operator()))(VersionSpec(ver));

        if (0 == r.op.compare(0, 1, "r"))
        {
            return (e.version.*(VersionOperator(vo_tilde).as_version_spec_operator()))(VersionSpec(ver)) &&
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

DepAtom::Pointer
PortageRepositorySets::security_set(bool insecurity) const
{
    Context context("When building security or insecurity package set:");
    AllDepAtom::Pointer security_packages(new AllDepAtom);

    if (!_imp->params.securitydir.is_directory())
        return security_packages;

    std::map<std::string, GLSADepTag::Pointer> glsa_tags;

    for (DirIterator f(_imp->params.securitydir), f_end ; f != f_end; ++f)
    {
        if (! IsFileWithExtension("glsa-", ".xml")(*f))
            continue;

        Context local_context("When parsing security advisory '" + stringify(*f) + "':");

        try
        {
            GLSA::ConstPointer glsa(GLSA::create_from_xml_file(stringify(*f)));
            Context local_local_context("When handling GLSA '" + glsa->id() + "' from '" +
                    stringify(*f) + "':");

            for (GLSA::PackagesIterator glsa_pkg(glsa->begin_packages()),
                    glsa_pkg_end(glsa->end_packages()) ; glsa_pkg != glsa_pkg_end ; ++glsa_pkg)
            {
                PackageDatabaseEntryCollection::ConstPointer candidates(_imp->environment->package_database()->query(
                            PackageDepAtom::Pointer(new PackageDepAtom(stringify(glsa_pkg->name()))),
                            insecurity ? is_either : is_installed_only));
                for (PackageDatabaseEntryCollection::Iterator c(candidates->begin()), c_end(candidates->end()) ;
                        c != c_end ; ++c)
                {
                    if (! is_vulnerable(*glsa_pkg, *c))
                        continue;

                    if (glsa_tags.end() == glsa_tags.find(glsa->id()))
                        glsa_tags.insert(std::make_pair(glsa->id(), GLSADepTag::Pointer(
                                        new GLSADepTag(glsa->id(), glsa->title()))));

                    if (insecurity)
                    {
                        PackageDepAtom::Pointer atom(new PackageDepAtom(
                                    "=" + stringify(c->name) + "-" + stringify(c->version) +
                                    "::" + stringify(c->repository)));
                        atom->set_tag(glsa_tags.find(glsa->id())->second);
                        security_packages->add_child(atom);
                    }
                    else
                    {
                        /* we need to find the best not vulnerable installable package that isn't masked
                         * that's in the same slot as our vulnerable installed package. */
                        bool ok(false);
                        SlotName wanted_slot(_imp->environment->package_database()->fetch_repository(
                                    c->repository)->version_metadata(c->name, c->version)->slot);

                        PackageDatabaseEntryCollection::ConstPointer available(
                                _imp->environment->package_database()->query(PackageDepAtom::Pointer(
                                        new PackageDepAtom(stringify(glsa_pkg->name()))), is_uninstalled_only));
                        for (PackageDatabaseEntryCollection::ReverseIterator r(available->rbegin()),
                                r_end(available->rend()) ; r != r_end ; ++r)
                        {
                            if (_imp->environment->mask_reasons(*r).any())
                                continue;
                            if (_imp->environment->package_database()->fetch_repository(r->repository)->version_metadata(
                                        r->name, r->version)->slot != wanted_slot)
                                continue;
                            if (is_vulnerable(*glsa_pkg, *r))
                                continue;

                            PackageDepAtom::Pointer atom(new PackageDepAtom(
                                        "=" + stringify(r->name) + "-" + stringify(r->version) +
                                        "::" + stringify(r->repository)));
                            atom->set_tag(glsa_tags.find(glsa->id())->second);
                            security_packages->add_child(atom);
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

