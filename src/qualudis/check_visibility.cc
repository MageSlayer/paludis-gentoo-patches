/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "check_visibility.hh"
#include "qa_notice.hh"

using namespace paludis;

class VisibilityChecker :
    public DepAtomVisitorTypes::ConstVisitor
{
    private:
        const Environment * const _env;
        bool _ok;
        std::list<std::string> _bad_deps;

    public:
        const bool & ok;
        const std::list<std::string> & bad_deps;

        VisibilityChecker(const Environment * const e) :
            _env(e),
            _ok(true),
            ok(_ok),
            bad_deps(_bad_deps)
        {
        }

        void visit(const AllDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * a)
        {
            bool at_least_one_match(false), local_ok(false);

            for (CompositeDepAtom::Iterator i(a->begin()), i_end(a->end()) ;
                    i != i_end ; ++i)
            {
                const UseDepAtom * p;
                if (((p = (*i)->as_use_dep_atom())))
                {
                    Save<bool> save_ok(&_ok, true);
                    accept_visitor(this)(i->raw_pointer());
                    local_ok |= ok;
                    at_least_one_match = true;
                }
                else
                {
                    Save<bool> save_ok(&_ok, true);
                    accept_visitor(this)(i->raw_pointer());
                    local_ok |= ok;
                    at_least_one_match = true;
                }
            }

            if (! ((! at_least_one_match) || local_ok))
                _ok = false;
        }

        void visit(const UseDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom *)
        {
        }

        void visit(const PackageDepAtom * a)
        {
            PackageDatabaseEntryCollection::ConstPointer m(0);
            if (((m = _env->package_database()->query(a)))->empty())
                _ok = false;
            else
            {
                _ok = false;
                for (PackageDatabaseEntryCollection::Iterator mm(m->begin()), mm_end(m->end()) ;
                        mm != mm_end ; ++mm)
                    if (! _env->mask_reasons(*mm).any())
                    {
                        _ok = true;
                        break;
                    }
            }

            if (! _ok)
                _bad_deps.push_back(stringify(*a));
        }

        void clear()
        {
            _ok = true;
            _bad_deps.clear();
        }
};

bool
check_visibility(const paludis::Environment * const env, const paludis::PackageDatabaseEntry & e)
{
    bool ok = true;

    VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(e));
    VisibilityChecker checker(env);

    DepParser::parse(metadata->get(vmk_depend))->accept(&checker);
    if (! checker.ok)
    {
        *QANotices::get_instance() << QANotice(qal_major, stringify(e),
                "Invisible DEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    DepParser::parse(metadata->get(vmk_rdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(qal_major, stringify(e),
                "Invisible RDEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    DepParser::parse(metadata->get(vmk_pdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(qal_major, stringify(e),
                "Invisible PDEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    return ok;
}

