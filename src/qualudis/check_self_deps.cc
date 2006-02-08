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

#include <paludis/paludis.hh>
#include <algorithm>
#include <list>

#include "check_self_deps.hh"
#include "qa_notice.hh"

using namespace paludis;

class SelfDepsChecker :
    public DepAtomVisitorTypes::ConstVisitor
{
    private:
        const Environment * const _env;
        const QualifiedPackageName _p;
        std::list<std::string> _bad_deps;
        bool _in_block;
        QANoticeLevel _level;

    public:
        const std::list<std::string> & bad_deps;
        const QANoticeLevel & level;

        SelfDepsChecker(const Environment * const e, const QualifiedPackageName & pp) :
            _env(e),
            _p(pp),
            _in_block(false),
            _level(qal_info),
            bad_deps(_bad_deps),
            level(_level)
        {
        }

        void visit(const AllDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const AnyDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const UseDepAtom * a)
        {
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }

        void visit(const BlockDepAtom * a)
        {
            Save<bool> save_in_block(&_in_block, true);
            a->blocked_atom()->accept(this);
        }

        void visit(const PackageDepAtom * a)
        {
            if (a->package() == _p)
            {
                if (_in_block)
                    _bad_deps.push_back("!" + stringify(*a));
                else
                    _bad_deps.push_back(stringify(*a));

                if (a->version_spec_ptr() || a->slot_ptr())
                    _level = std::max(_level, qal_maybe);
                else
                    _level = std::max(_level, qal_major);
            }
        }

        void clear()
        {
            _bad_deps.clear();
            _level = qal_info;
        }
};

int
check_self_deps(const Environment * const env, const PackageDatabaseEntry & e)
{
    int ret_code(0);
    VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(e));
    SelfDepsChecker checker(env, e.get<pde_name>());

    DepParser::parse(metadata->get(vmk_depend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "DEPEND self circular: '" + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ret_code |= 1;
    }

    DepParser::parse(metadata->get(vmk_rdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "RDEPEND self circular: '" + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ret_code |= 1;
    }

    DepParser::parse(metadata->get(vmk_pdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "PDEPEND self circular: '" + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ret_code |= 1;
    }

    return ret_code;
}

