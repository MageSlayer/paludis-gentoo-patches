/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "dependency_keys.hh"
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/qa.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Checker :
        ConstVisitor<DependencySpecTree>
    {
        QAReporter & reporter;
        const tr1::shared_ptr<const PackageID> & id;
        const MetadataSpecTreeKey<DependencySpecTree> & key;
        const std::string name;
        unsigned level;

        Checker(QAReporter & r,
                const tr1::shared_ptr<const PackageID> & i,
                const MetadataSpecTreeKey<DependencySpecTree> & k,
                const std::string & n) :
            reporter(r),
            id(i),
            key(k),
            name(n),
            level(0)
        {
        }

        void visit_leaf(const PackageDepSpec &)
        {
        }

        void visit_leaf(const BlockDepSpec &)
        {
        }

        void visit_sequence(const UseDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<unsigned> save_level(&level, level + 1);
            if (cur == end)
                reporter.message(qaml_normal, name, "Empty 'use? ( )' block in dependency key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
            else
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AllDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<unsigned> save_level(&level, level + 1);
            if (cur == end)
            {
                if (level > 1)
                    reporter.message(qaml_normal, name, "Empty '( )' block in dependency key '" + stringify(key.raw_name())
                            + "' for ID '" + stringify(*id) + "'");
            }
            else
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AnyDepSpec &,
                DependencySpecTree::ConstSequenceIterator cur,
                DependencySpecTree::ConstSequenceIterator end)
        {
            Save<unsigned> save_level(&level, level + 1);
            if (cur == end)
                reporter.message(qaml_normal, name, "Empty '|| ( )' block in dependency key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
            else
                std::for_each(cur, end, accept_visitor(*this));
        }
    };

    bool dependency_key_check(
            QAReporter & reporter,
            const tr1::shared_ptr<const PackageID> & id,
            const std::string & name,
            const MetadataSpecTreeKey<DependencySpecTree> & key)
    {
        Context context("When checking dependency key '" + key.raw_name() + "' for check '" + name +
                "' using dependency_keys_check on ID '" + stringify(*id) + "':");

        Checker c(reporter, id, key, name);
        key.value()->accept(c);

        return true;
    }
}

bool
paludis::erepository::dependency_keys_check(
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using dependency_keys_check on ID '" + stringify(*id) + "':");

    bool result(true);

    if (id->build_dependencies_key())
        result &= dependency_key_check(reporter, id, name, *id->build_dependencies_key());

    if (id->run_dependencies_key())
        result &= dependency_key_check(reporter, id, name, *id->run_dependencies_key());

    if (id->post_dependencies_key())
        result &= dependency_key_check(reporter, id, name, *id->post_dependencies_key());

    if (id->suggested_dependencies_key())
        result &= dependency_key_check(reporter, id, name, *id->suggested_dependencies_key());

    return result;
}

