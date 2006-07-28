/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_list.hh>
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

using namespace paludis;

namespace
{
    class AdvisoryVisitor :
        private InstantiationPolicy<AdvisoryVisitor, instantiation_method::NonCopyableTag>,
        public DepAtomVisitorTypes::ConstVisitor
    {
        private:
            const Environment * const _env;

            mutable const CompositeDepAtom & _a;

            mutable std::vector<const PackageDepAtom *> _atoms;

        protected:
            ///\name Visit methods
            ///{
            void visit(const AllDepAtom *);
            void visit(const AnyDepAtom *) PALUDIS_ATTRIBUTE((noreturn));
            void visit(const UseDepAtom *);
            void visit(const PlainTextDepAtom *);
            void visit(const PackageDepAtom *);
            void visit(const BlockDepAtom *);
            ///}

        public:
            /**
             * Constructor.
             */
            AdvisoryVisitor(const Environment * const env, const CompositeDepAtom & a);

            /**
             * Destructor.
             */
            ~AdvisoryVisitor()
            {
            }

            /**
             * Iterate over our dep atoms.
             */
            typedef std::vector<const PackageDepAtom *>::iterator Iterator;

            /**
             * Grab element by index.
             */
            const PackageDepAtom * at(std::vector<const PackageDepAtom *>::size_type n) const
            {
                return _atoms[n];
            }

            /**
             * Return the number of atoms.
             */
            std::vector<const PackageDepAtom *>::size_type size() const
            {
                return _atoms.size();
            }
    };
}

AdvisoryVisitor::AdvisoryVisitor(const Environment * const env, const CompositeDepAtom & a) :
    _env(env),
    _a(a)
{
    Context c("When flattening the AdvisoryFile line:");
    std::for_each(a.begin(), a.end(), accept_visitor(this));
    if (_atoms.size() == 2)
    {
        VersionOperatorValue v1(_atoms[0]->version_operator().value()),
                v2(_atoms[1]->version_operator().value());

        if ((v1 == vo_equal) || (v2 == vo_equal))
            throw AdvisoryFileError("Broken line: Forbidden 'equal' atom in range");
    }
}

void
AdvisoryVisitor::visit(const AllDepAtom * a)
{
    std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
AdvisoryVisitor::visit(const AnyDepAtom *)
{
    throw AdvisoryFileError("Unexpected AnyDepAtom in line");
}

void
AdvisoryVisitor::visit(const UseDepAtom * a)
{
    if (_env->query_use(a->flag(), 0) ^ a->inverse())
        std::for_each(a->begin(), a->end(), accept_visitor(this));
}

void
AdvisoryVisitor::visit(const PackageDepAtom * a)
{
    _atoms.push_back(a);
}

void
AdvisoryVisitor::visit(const PlainTextDepAtom *)
{
}

void
AdvisoryVisitor::visit(const BlockDepAtom *)
{
}


namespace paludis
{
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
PortageRepositorySets::package_set(const std::string & s, const PackageSetOptions & o) const
{
    if ("system" == s)
        throw InternalError(PALUDIS_HERE, "system set should've been handled by PortageRepository");
    else if ("security" == s)
        return security_set(o);
    else if ((_imp->params.get<prpk_setsdir>() / (s + ".conf")).exists())
    {
        GeneralSetDepTag::Pointer tag(new GeneralSetDepTag(s));

        FSEntry ff(_imp->params.get<prpk_setsdir>() / (s + ".conf"));
        Context context("When loading package set '" + s + "' from '" + stringify(ff) + "':");

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

namespace
{
    inline
    PackageDepAtom::Pointer make_atom(const PackageDatabaseEntry & e)
    {
        QualifiedPackageName n(e.get<pde_name>());
        VersionSpec v(e.get<pde_version>());

        std::string s("=" + stringify(n) + "-" + stringify(v));
        return PackageDepAtom::Pointer(new PackageDepAtom(s));
    }
}

PackageDatabaseEntryCollection::Iterator
PortageRepositorySets::find_best(PackageDatabaseEntryCollection & c, const PackageDatabaseEntry & e) const
{
    Context local("When finding best update for '" + stringify(e.get<pde_name>()) + "-" +
            stringify(e.get<pde_version>()) + "':");
    // Find an entry in c that matches e best. e is not in c.
    QualifiedPackageName n(e.get<pde_name>());
    SlotName s(_imp->environment->package_database()->fetch_repository(
                e.get<pde_repository>())->version_metadata(e.get<pde_name>(), e.get<pde_version>())->get<vm_slot>());
    PackageDatabaseEntryCollection::Iterator i(c.begin()), i_end(c.end()), i_best(c.end());
    for ( ; i != i_end; ++i)
    {
        if (n != i->get<pde_name>())
            continue;
        if (s != _imp->environment->package_database()->fetch_repository(
                    i->get<pde_repository>())->version_metadata(
                    i->get<pde_name>(), i->get<pde_version>())->get<vm_slot>())
            continue;

        i_best = i;
    }

    return i_best;
}


DepAtom::Pointer
PortageRepositorySets::security_set(const PackageSetOptions & o) const
{
    Context c("When building security package set:");
    AllDepAtom::Pointer security_packages(new AllDepAtom);

    bool list_affected_only(o.get<pso_list_affected_only>());
    InstallState affected_state(list_affected_only ? is_either : is_installed_only);

    if (!_imp->params.get<prpk_securitydir>().is_directory())
        return DepAtom::Pointer(new AllDepAtom);

    std::list<FSEntry> advisories;
    std::copy(DirIterator(_imp->params.get<prpk_securitydir>()), DirIterator(),
        filter_inserter(std::back_inserter(advisories),
        IsFileWithExtension("advisory-", ".conf")));

    std::list<FSEntry>::const_iterator f(advisories.begin()),
        f_end(advisories.end());

    std::set<std::pair<PackageDatabaseEntry, std::string> > affected;
    PackageDatabaseEntryCollection::Concrete unaffected;
    std::map<std::string, std::string> advisory_map;

    for ( ; f != f_end; ++f)
    {
        Context c("When parsing security advisory '" + stringify(*f) + "':");

        try
        {
            AdvisoryFile advisory(*f);
            std::string advisory_id(advisory.get("Id"));
            advisory_map[advisory_id] = advisory.get("Title");


            AdvisoryFile::LineIterator a(advisory.begin_affected()), a_end(advisory.end_affected());
            for ( ; a != a_end ; ++a)
            {
                Context c("When parsing line 'Affected: " + *a + "':");

                CompositeDepAtom::ConstPointer line(PortageDepParser::parse(*a));
                AdvisoryVisitor atoms(_imp->environment, *line);

                if ((0 == atoms.size()) || (2 < atoms.size()))
                {
                    continue;
                }

                bool is_range(2 == atoms.size());

                PackageDatabaseEntryCollection::ConstPointer affected_collection1(
                        _imp->environment->package_database()->query(*atoms.at(0), affected_state));
                PackageDatabaseEntryCollection::ConstPointer affected_collection2(
                        new PackageDatabaseEntryCollection::Concrete);
                PackageDatabaseEntryCollection::Iterator p(affected_collection1->begin()),
                    p_end(affected_collection1->end());

                if (is_range)
                    affected_collection2 = _imp->environment->package_database()->query(
                            *atoms.at(1), affected_state);

                for ( ; p != p_end ; ++p)
                {
                    if ((affected.end() != affected.find(std::make_pair(*p, advisory_id))))
                        continue;
                    if ((! is_range) || (affected_collection2->end() != affected_collection2->find(*p)))
                        affected.insert(std::make_pair(*p, advisory_id));
                }
            }

            AdvisoryFile::LineIterator u(advisory.begin_unaffected()), u_end(advisory.end_unaffected());
            for ( ; u != u_end ; ++u)
            {
                Context c("When parsing line 'Unaffected: " + *u + "':");

                CompositeDepAtom::ConstPointer line(PortageDepParser::parse(*u));
                AdvisoryVisitor atoms(_imp->environment, *line);

                if ((0 == atoms.size()) || (2 < atoms.size()))
                {
                    continue;
                }

                bool is_range(2 == atoms.size());

                PackageDatabaseEntryCollection::ConstPointer unaffected_collection1(
                        _imp->environment->package_database()->query(*atoms.at(0), is_either));
                PackageDatabaseEntryCollection::ConstPointer unaffected_collection2(
                        new PackageDatabaseEntryCollection::Concrete);
                PackageDatabaseEntryCollection::Iterator p(unaffected_collection1->begin()),
                    p_end(unaffected_collection1->end());

                if (is_range)
                    unaffected_collection2 = _imp->environment->package_database()->query(
                            *atoms.at(1), is_either);

                for ( ; p != p_end ; ++p)
                {
                    if ((! is_range) || (unaffected_collection2->end() != unaffected_collection2->find(*p)))
                    {
                        unaffected.insert(*p);
                        std::set<std::pair<PackageDatabaseEntry, std::string> >::iterator
                                a(affected.find(std::make_pair(*p, advisory_id)));
                        if (a != affected.end())
                            affected.erase(a);
                    }
                }
            }
        }
        catch (const AdvisoryFileError & e)
        {
            Log::get_instance()->message(ll_warning, lc_context,
                    "Malformed advisory file '" + stringify(*f) + "': " + e.message());
        }
        catch (const InternalError & e)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context,
                    "Exception caught while parsing advisory '" + stringify(*f) +
                    "': " + e.message());
        }

    }

    std::set<std::pair<PackageDatabaseEntry, std::string> >::const_iterator
        i(affected.begin()), i_end(affected.end());
    if (list_affected_only)
    {
        for ( ; i != i_end ; ++i)
        {
            Context c("When creating adding vulnerable package '" + stringify(i->first) + "':");

            PackageDepAtom::Pointer p(make_atom(i->first));
            p->set_tag(GLSADepTag::Pointer(new GLSADepTag(i->second, advisory_map[i->second])));
            security_packages->add_child(p);
        }
    }
    else
    {
        for ( ; i != i_end ; ++i)
        {
            Context c("When finding best update for package '" + stringify(i->first) +
                    "', affected by '" + i->second + "':");

            PackageDatabaseEntryCollection::Iterator best = find_best(unaffected, i->first);
            if (best == unaffected.end())
                throw AllMaskedError("No best update available for package '" + stringify(i->first) + "':");

            PackageDepAtom::Pointer p(make_atom(*best));
            p->set_tag(GLSADepTag::Pointer(new GLSADepTag(i->second, advisory_map[i->second])));
            security_packages->add_child(p);
        }
    }

    return security_packages;
}

