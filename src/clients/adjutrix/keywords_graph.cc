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

#include "keywords_graph.hh"
#include "command_line.hh"
#include <output/colour.hh>

#include <paludis/tasks/find_unused_packages_task.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/query.hh>
#include <paludis/repository.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>

#include <set>
#include <map>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace paludis;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
    template <typename T_>
    struct CompareByStringLength :
        std::binary_function<const T_ &, const T_ &, bool>
    {
        bool
        operator() (const T_ & l, const T_ & r) const
        {
            return stringify(l).length() < stringify(r).length();
        }
    };

    void
    write_keywords_graph(const Environment & e, const Repository & repo,
            const QualifiedPackageName & package)
    {
        using namespace tr1::placeholders;

        Context context("When writing keyword graph for '" + stringify(package) + "' in '"
                + stringify(repo.name()) + "':");

        cout << "Keywords for " << package << ":" << endl;
        cout << endl;

        FindUnusedPackagesTask task(&e, &repo);
        tr1::shared_ptr<const PackageIDSequence> packages(e.package_database()->query(
                query::Matches(PackageDepSpec(
                        tr1::shared_ptr<QualifiedPackageName>(new QualifiedPackageName(package)),
                        tr1::shared_ptr<CategoryNamePart>(),
                        tr1::shared_ptr<PackageNamePart>(),
                        tr1::shared_ptr<VersionRequirements>(),
                        vr_and,
                        tr1::shared_ptr<SlotName>(),
                        tr1::shared_ptr<RepositoryName>(new RepositoryName(repo.name())))),
                qo_group_by_slot));
        tr1::shared_ptr<const PackageIDSequence> unused(task.execute(package));

        if (packages->empty())
            return;

        if (! repo.use_interface)
            throw InternalError(PALUDIS_HERE, "Repository has no use_interface");

        tr1::shared_ptr<const UseFlagNameSet> arch_flags(repo.use_interface->arch_flags());
        if (arch_flags->empty())
            return;

        std::set<SlotName> slots;
        std::copy(packages->begin(), packages->end(),
                transform_inserter(std::inserter(slots, slots.begin()), tr1::mem_fn(&PackageID::slot)));

        unsigned version_specs_columns_width(std::max_element(indirect_iterator(packages->begin()),
                    indirect_iterator(packages->end()),
                    tr1::bind(CompareByStringLength<std::string>(),
                        tr1::bind(tr1::mem_fn(&PackageID::canonical_form), _1, idcf_version),
                        tr1::bind(tr1::mem_fn(&PackageID::canonical_form), _2, idcf_version))
                    )->canonical_form(idcf_version).length() + 1);

        unsigned tallest_arch_name(std::max(stringify(*std::max_element(arch_flags->begin(),
                            arch_flags->end(), CompareByStringLength<UseFlagName>())).length(), static_cast<std::size_t>(6)));

        unsigned longest_slot_name(stringify(*std::max_element(slots.begin(),
                        slots.end(), CompareByStringLength<SlotName>())).length());

        for (unsigned h = 0 ; h < tallest_arch_name ; ++h)
        {
            cout << std::left << std::setw(version_specs_columns_width) << " " << "| ";
            for (UseFlagNameSet::Iterator a(arch_flags->begin()), a_end(arch_flags->end()) ;
                    a != a_end ; ++a)
            {
                if ((tallest_arch_name - h) > a->data().length())
                    cout << "  ";
                else
                    cout << a->data().at(a->data().length() - tallest_arch_name + h) << " ";
            }
            cout << "| ";
            if ((tallest_arch_name - h) <= 6)
                cout << std::string("unused").at(6 - tallest_arch_name + h);
            else
                cout << " ";

            cout << " ";
            if ((tallest_arch_name - h) <= 4)
                cout << std::string("slot").at(4 - tallest_arch_name + h);

            cout << endl;
        }

        cout << std::string(version_specs_columns_width, '-') << "+"
            << std::string(arch_flags->size() * 2 + 1, '-') << "+"
            << std::string(longest_slot_name + 3, '-') << endl;

        SlotName old_slot("first_slot");
        for (IndirectIterator<PackageIDSequence::Iterator> p(packages->begin()), p_end(packages->end()) ;
                p != p_end ; ++p)
        {
            if (! p->keywords_key())
                continue;

            if (p->slot() != old_slot)
                if (old_slot != SlotName("first_slot"))
                    cout << std::string(version_specs_columns_width, '-') << "+"
                        << std::string(arch_flags->size() * 2 + 1, '-') << "+"
                        << std::string(longest_slot_name + 3, '-') << endl;

            cout << std::left << std::setw(version_specs_columns_width) << p->canonical_form(idcf_version) << "| ";

            tr1::shared_ptr<const KeywordNameSet> keywords(p->keywords_key()->value());

            for (UseFlagNameSet::Iterator a(arch_flags->begin()), a_end(arch_flags->end()) ;
                    a != a_end ; ++a)
            {
                if (keywords->end() != keywords->find(KeywordName(stringify(*a))))
                    cout << colour(cl_bold_green, "+ ");
                else if (keywords->end() != keywords->find(KeywordName("~" + stringify(*a))))
                    cout << colour(cl_bold_yellow, "~ ");
                else if (keywords->end() != keywords->find(KeywordName("-" + stringify(*a))))
                    cout << colour(cl_red, "- ");
                else if (keywords->end() != keywords->find(KeywordName("-*")))
                    cout << colour(cl_red, "* ");
                else
                    cout << "  ";
            }

            cout << "| " << (indirect_iterator(unused->end()) !=
                    std::find(indirect_iterator(unused->begin()), indirect_iterator(unused->end()), *p) ? "* " : "  ");

            if (p->slot() != old_slot)
            {
                cout << p->slot();
                old_slot = p->slot();
            }

            cout << endl;
        }

        cout << endl;
    }
}

void do_keywords_graph(const NoConfigEnvironment & env)
{
    Context context("When performing keywords-graph action:");

    for (IndirectIterator<PackageDatabase::RepositoryIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

        tr1::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::Iterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            tr1::shared_ptr<const QualifiedPackageNameSet> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameSet::Iterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
            {
                if (CommandLine::get_instance()->a_package.specified())
                    if (CommandLine::get_instance()->a_package.end_args() == std::find(
                                CommandLine::get_instance()->a_package.begin_args(),
                                CommandLine::get_instance()->a_package.end_args(),
                                stringify(p->package)))
                        continue;

                write_keywords_graph(env, *r, *p);
            }
        }
    }
}

