/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/qa/deps_visible_check.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/repositories/portage/portage_repository.hh>
#include <paludis/environment.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/dep_atom.hh>
#include <paludis/dep_atom_pretty_printer.hh>
#include <paludis/util/save.hh>
#include <paludis/query.hh>

#include <list>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct IsViableAnyDepAtomChild
    {
        const QAEnvironment * const env;
        const PackageDatabaseEntry pde;

        IsViableAnyDepAtomChild(const QAEnvironment * const e, const PackageDatabaseEntry & p) :
            env(e),
            pde(p)
        {
        }

        template <typename T_>
        bool operator() (T_ atom) const
        {
            const UseDepAtom * const u(atom->as_use_dep_atom());
            if (0 != u)
            {
                RepositoryUseInterface * i(env->package_database()->fetch_repository(
                            pde.repository)->use_interface);
                if (! i)
                    return true;

                if (i->query_use_mask(u->flag(), &pde) && ! u->inverse())
                    return false;

                if (i->query_use_force(u->flag(), &pde) && u->inverse())
                    return false;

                /* arch flags aren't necessarily use masked. stupid! */
                std::tr1::shared_ptr<const UseFlagNameCollection> arch_flags(i->arch_flags());
                if (stringify(u->flag()) != env->portage_repository()->profile_variable("ARCH"))
                    if (arch_flags->end() != arch_flags->find(u->flag()))
                        return u->inverse();

                return true;
            }
            else
                return true;
        }
    };

    struct Checker :
        DepAtomVisitorTypes::ConstVisitor,
        DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepAtom>
    {
        using DepAtomVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepAtom>::visit;

        CheckResult & result;
        const std::string role;
        const QAEnvironment * env;
        const PackageDatabaseEntry & pde;
        bool unstable;

        Checker(CheckResult & rr, const std::string & r, const QAEnvironment * e,
                const PackageDatabaseEntry & p, bool u) :
            result(rr),
            role(r),
            env(e),
            pde(p),
            unstable(u)
        {
        }

        void visit(const PackageDepAtom * const p)
        {
            bool found(false);
            std::string candidates;
            std::tr1::shared_ptr<PackageDatabaseEntryCollection> matches(env->package_database()->query(
                        query::Matches(*p), qo_order_by_version));
            for (PackageDatabaseEntryCollection::ReverseIterator m(matches->rbegin()),
                    m_end(matches->rend()) ; m != m_end ; ++m)
            {
                MaskReasons r;
                if (((r = env->mask_reasons(*m))).any())
                {
                    if (! candidates.empty())
                        candidates.append(", ");
                    candidates.append(stringify(m->version));
                    candidates.append(":");
                    for (MaskReason rr(MaskReason(0)) ; rr < last_mr ;
                            rr = MaskReason(static_cast<int>(rr) + 1))
                        if (r[rr])
                            candidates.append(" " + stringify(rr));
                    continue;
                }

                found = true;
                break;
            }

            if (! found)
                result << Message(qal_major, "No visible provider for " + role + " entry '"
                        + stringify(*p) + "'" + (unstable ? " (unstable)" : "") + " (candidates: "
                        + candidates + ")");
        }

        void visit(const AnyDepAtom * const a)
        {
            std::list<std::tr1::shared_ptr<const DepAtom> > viable_children;
            std::copy(a->begin(), a->end(), filter_inserter(std::back_inserter(viable_children),
                        IsViableAnyDepAtomChild(env, pde)));

            if (viable_children.empty())
                return;

            bool found(false);
            for (std::list<std::tr1::shared_ptr<const DepAtom> >::const_iterator c(viable_children.begin()),
                    c_end(viable_children.end()) ; c != c_end && ! found ; ++c)
            {
                Save<CheckResult> save_result(&result);
                result.clear();
                (*c)->accept(this);
                if (result.empty())
                    found = true;
            }

            if (! found)
            {
                DepAtomPrettyPrinter printer(0, false);
                a->accept(&printer);
                result << Message(qal_major, "No visible provider for " + role + " entry '"
                        + stringify(printer) + "'" + (unstable ? " (unstable)" : ""));
            }
        }

        void visit(const UseDepAtom * const u)
        {
            if (IsViableAnyDepAtomChild(env, pde)(u))
                std::for_each(u->begin(), u->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * const)
        {
        }

        void visit(const PlainTextDepAtom * const)
        {
        }
    };
}

DepsVisibleCheck::DepsVisibleCheck()
{
}

CheckResult
DepsVisibleCheck::operator() (const PerProfileEbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        e.environment->portage_repository()->set_profile(
                e.environment->portage_repository()->find_profile(e.profile));
        if (e.environment->master_repository())
            e.environment->master_repository()->set_profile(
                    e.environment->master_repository()->find_profile(e.profile));

        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->portage_repository()->name());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        bool unstable(false);
        do
        {
            e.environment->set_accept_unstable(unstable);

            if (e.environment->mask_reasons(ee).any())
                result << Message(qal_skip, "Masked, so skipping checks");
            else
            {
                Checker depend_checker(result, "DEPEND", e.environment, ee, unstable);
                std::string depend(metadata->deps_interface->build_depend_string);
                PortageDepParser::parse(depend)->accept(&depend_checker);

                Checker rdepend_checker(result, "RDEPEND", e.environment, ee, unstable);
                std::string rdepend(metadata->deps_interface->run_depend_string);
                PortageDepParser::parse(rdepend)->accept(&rdepend_checker);

                Checker pdepend_checker(result, "PDEPEND", e.environment, ee, unstable);
                std::string pdepend(metadata->deps_interface->post_depend_string);
                PortageDepParser::parse(pdepend)->accept(&pdepend_checker);
            }

            if (unstable)
                break;
            else
                unstable = true;

        } while (true);
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & err)
    {
        result << Message(qal_fatal, "Caught Exception '" + err.message() + "' ("
                + err.what() + ")");
    }

    return result;
}

const std::string &
DepsVisibleCheck::identifier()
{
    static const std::string id("deps_visible");
    return id;
}

