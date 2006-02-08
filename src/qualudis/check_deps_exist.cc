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
#include <iostream>
#include <algorithm>
#include <list>

#include "check_deps_exist.hh"

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

struct DepExistsChecker :
    DepAtomVisitorTypes::ConstVisitor
{
    const Environment * const env;
    std::list<std::string> bad_deps;
    bool in_block;

    DepExistsChecker(const Environment * const e) :
        env(e),
        in_block(false)
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
        Save<bool> save_in_block(&in_block, true);
        a->blocked_atom()->accept(this);
    }

    void visit(const PackageDepAtom * a)
    {
        if (env->package_database()->query(a)->empty())
        {
            if (in_block)
                bad_deps.push_back("!" + stringify(*a));
            else
                bad_deps.push_back(stringify(*a));
        }
    }
};

int
check_deps_exist(const Environment * const env, const PackageDatabaseEntry & e)
{
    int ret_code(0);
    VersionMetadata::ConstPointer metadata(env->package_database()->fetch_metadata(e));
    DepExistsChecker checker(env);

    DepParser::parse(metadata->get(vmk_depend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        cout << "[WARNING] In DEPEND for '" << e << "', these entries do not "
            "exist: '" << join(checker.bad_deps.begin(), checker.bad_deps.end(),
                    "', '") << "'" << endl;
        checker.bad_deps.clear();
        ret_code |= 1;
    }

    DepParser::parse(metadata->get(vmk_rdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        cout << "[WARNING] In RDEPEND for '" << e << "', these entries do not "
            "exist: '" << join(checker.bad_deps.begin(), checker.bad_deps.end(),
                    "', '") << "'" << endl;
        checker.bad_deps.clear();
        ret_code |= 1;
    }

    DepParser::parse(metadata->get(vmk_pdepend))->accept(&checker);
    if (! checker.bad_deps.empty())
    {
        cout << "[WARNING] In PDEPEND for '" << e << "', these entries do not "
            "exist: '" << join(checker.bad_deps.begin(), checker.bad_deps.end(),
                    "', '") << "'" << endl;
        checker.bad_deps.clear();
        ret_code |= 1;
    }

    return ret_code;
}

