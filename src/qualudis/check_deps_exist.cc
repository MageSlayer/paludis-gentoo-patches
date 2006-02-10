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

#include "check_deps_exist.hh"
#include "qa_notice.hh"

using namespace paludis;

class DepExistsChecker :
    public DepAtomVisitorTypes::ConstVisitor
{
    private:
        const Environment * const _env;
        std::list<std::string> _bad_deps;
        bool _in_block;
        bool _in_any_of;
        bool _at_least_one_ok;
        QANoticeLevel _level;

    public:
        const std::list<std::string> & bad_deps;
        const QANoticeLevel & level;

        DepExistsChecker(const Environment * const e) :
            _env(e),
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
            Save<bool> save_in_any_of(&_in_any_of, true);
            Save<bool> save_at_least_one_ok(&_at_least_one_ok, false);
            std::for_each(a->begin(), a->end(), accept_visitor(this));

            if (a->begin() != a->end() && ! _at_least_one_ok)
                _level = std::max(level, qal_major);
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
            if (_env->package_database()->query(a)->empty())
            {
                if (_in_block)
                {
                    _bad_deps.push_back("!" + stringify(*a));
                    _level = std::max(_level, qal_maybe);
                }
                else if (_in_any_of)
                {
                    _bad_deps.push_back(stringify(*a));
                    _level = std::max(_level, qal_maybe);
                }
                else
                {
                    _bad_deps.push_back(stringify(*a));
                    _level = std::max(_level, qal_major);
                }
            }
            else
                _at_least_one_ok = true;
        }

        void clear()
        {
            _bad_deps.clear();
            _level = qal_info;
        }
};

bool
check_deps_exist(const Environment * const env, const PackageDatabaseEntry & e)
{
    bool ok(true);
    VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(e));
    DepExistsChecker checker(env);

    DepParser::parse(metadata->get(vmk_depend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "Nonexistent DEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    DepParser::parse(metadata->get(vmk_rdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "Nonexistent RDEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    DepParser::parse(metadata->get(vmk_pdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        *QANotices::get_instance() << QANotice(checker.level, stringify(e),
                "Nonexistent PDEPEND entries: " + join(checker.bad_deps.begin(),
                    checker.bad_deps.end(), "', '") + "'");
        checker.clear();
        ok = false;
    }

    return ok;
}

