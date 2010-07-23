/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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

#include "find_reverse_deps.hh"
#include "command_line.hh"
#include <output/colour.hh>

#include <paludis/util/save.hh>
#include <paludis/util/log.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/dep_spec.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_id.hh>
#include <paludis/package_database.hh>
#include <paludis/metadata_key.hh>
#include <paludis/fuzzy_finder.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>

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
    class ReverseDepChecker
    {
        private:
            const Environment * const _env;
            const PackageIDSequence & _entries;
            std::string _depname;
            std::string _p;

            bool _in_any;
            bool _in_use;
            std::string _flags;

            bool _found_matches;

            std::set<SetName> _recursing_sets;

        public:
            ReverseDepChecker(const Environment * const e,
                    const PackageIDSequence & entries,
                    const std::string & p) :
                _env(e),
                _entries(entries),
                _depname(""),
                _p(p),
                _in_any(false),
                _in_use(false),
                _found_matches(false)
            {
            }

            void check(const std::shared_ptr<const DependencySpecTree> & tree, const std::string & depname)
            {
                _depname = depname;
                tree->root()->accept(*this);
            }

            bool found_matches()
            {
                return _found_matches;
            }

            void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node);
            void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node);

            void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
            {
            }

            void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
            {
            }

            void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type & node)
            {
                Context context("When expanding named set '" + stringify(*node.spec()) + "':");

                std::shared_ptr<const SetSpecTree> set(_env->set(node.spec()->name()));

                if (! set)
                {
                    Log::get_instance()->message("adjutrix.find_reverse_deps.unknown_set", ll_warning, lc_context)
                        << "Unknown set '" << node.spec()->name() << "'";
                    return;
                }

                if (! _recursing_sets.insert(node.spec()->name()).second)
                {
                    Log::get_instance()->message("adjutrix.find_reverse_deps.recursive_set", ll_warning, lc_context)
                        << "Recursively defined set '" << node.spec()->name() << "'";
                    return;
                }

                set->root()->accept(*this);

                _recursing_sets.erase(node.spec()->name());
            }
    };

    void
    ReverseDepChecker::visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
    {
        Save<bool> in_any_save(&_in_any, true);
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    void
    ReverseDepChecker::visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
    {
        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    void
    ReverseDepChecker::visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
    {
        Save<bool> in_use_save(&_in_use, true);
        Save<std::string> flag_save(&_flags);

        if (! _flags.empty())
            _flags += " ";
        _flags += stringify(*node.spec());

        std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
    }

    void
        ReverseDepChecker::visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
    {
        std::shared_ptr<const PackageIDSequence> dep_entries((*_env)[selection::AllVersionsSorted(
                    generator::Matches(*node.spec(), MatchPackageOptions() + mpo_ignore_additional_requirements))]);
        std::shared_ptr<PackageIDSequence> matches(std::make_shared<PackageIDSequence>());

        bool header_written = false;

        for (IndirectIterator<PackageIDSequence::ConstIterator> e(dep_entries->begin()), e_end(dep_entries->end()) ;
                e != e_end ; ++e)
        {
            if (indirect_iterator(_entries.end()) != std::find(indirect_iterator(_entries.begin()),
                        indirect_iterator(_entries.end()), *e))
            {
                _found_matches |= true;

                if (! header_written)
                {
                    std::cout << "  " << _p << " " + _depname + " on one of:" << std::endl;
                    header_written = true;
                }
                std::cout << "    " << stringify(*e);

                if (_in_use || _in_any)
                {
                    std::cout << " (";

                    if (_in_any)
                        std::cout << "any-of";

                    if (_in_use && _in_any)
                        std::cout << ", ";

                    if (_in_use)
                        std::cout << "condition USE='" << _flags << "'";

                    std::cout << ")";
                }
                std::cout << std::endl;
            }
        }
    }

    void write_repository_header(std::string spec, const std::string &)
    {
        cout << "Reverse dependencies for '" << spec << "':" << std::endl;
    }

    int check_one_package(const Environment & env,
            const PackageIDSequence & entries, const QualifiedPackageName & p)
    {
        Context context("When checking package '" + stringify(p) + "':");

        std::shared_ptr<const PackageIDSequence> p_entries(env[selection::AllVersionsSorted(generator::Package(p))]);

        bool found_matches(false);

        for (IndirectIterator<PackageIDSequence::ConstIterator> e(p_entries->begin()), e_end(p_entries->end()) ;
                e != e_end ; ++e)
        {
            try
            {
                ReverseDepChecker checker(&env, entries, stringify(p) + "-" + stringify(e->canonical_form(idcf_version)));

                if (e->build_dependencies_key())
                    checker.check(e->build_dependencies_key()->value(), e->build_dependencies_key()->raw_name());

                if (e->run_dependencies_key())
                    checker.check(e->run_dependencies_key()->value(), e->run_dependencies_key()->raw_name());

                if (e->post_dependencies_key())
                    checker.check(e->post_dependencies_key()->value(), e->post_dependencies_key()->raw_name());

                if (e->suggested_dependencies_key())
                    checker.check(e->suggested_dependencies_key()->value(), e->suggested_dependencies_key()->raw_name());

                found_matches |= checker.found_matches();
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & exception)
            {
                cerr << "Caught exception:" << endl;
                cerr << "  * " << exception.backtrace("\n  * ") << endl;
                cerr << "  * " << exception.message() << " (" << exception.what() << ")" << endl;
                return (found_matches ? 0 : 1) | 2;
            }
        }

        return found_matches ? 0 : 1;
    }
}

int do_find_reverse_deps(NoConfigEnvironment & env)
{
    Context context("When performing find-reverse-deps action:");

    std::shared_ptr<PackageDepSpec> spec;
    try
    {
        spec = std::make_shared<PackageDepSpec>(parse_user_package_dep_spec(*CommandLine::get_instance()->begin_parameters(),
                        &env, UserPackageDepSpecOptions()));
    }
    catch (const AmbiguousPackageNameError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "Ambiguous package name '" << e.name() << "'. Did you mean:" << endl;
        for (AmbiguousPackageNameError::OptionsConstIterator o(e.begin_options()),
                o_end(e.end_options()) ; o != o_end ; ++o)
            cerr << "    * " << colour(cl_package_name, *o) << endl;
        cerr << endl;
        return 4;
    }
    catch (const NoSuchPackageError & e)
    {
        cout << endl;
        cerr << "Query error:" << endl;
        cerr << "  * " << e.backtrace("\n  * ");
        cerr << "Could not find '" << e.name() << "'.";

        if (! CommandLine::get_instance()->a_no_suggestions.specified())
        {
            cerr << " Looking for suggestions:" << endl;

            FuzzyCandidatesFinder f(env, e.name(), filter::All());

            if (f.begin() == f.end())
                cerr << "No suggestions found." << endl;
            else
                cerr << "Suggestions:" << endl;

            for (FuzzyCandidatesFinder::CandidatesConstIterator c(f.begin()),
                     c_end(f.end()) ; c != c_end ; ++c)
                cerr << "  * " << colour(cl_package_name, *c) << endl;
        }

        cerr << endl;
        return 5;
    }

    std::shared_ptr<const PackageIDSequence> entries(env[selection::AllVersionsSorted(generator::Matches(
                    *spec, MatchPackageOptions()))]);
    int ret(0);

    if (entries->empty())
    {
        Log::get_instance()->message("adjutrix.find_reverse_deps.no_matches", ll_warning, lc_context)
            << "No matches in package database for '" << *spec << "'";
        return 1;
    }

    for (IndirectIterator<PackageDatabase::RepositoryConstIterator, const Repository>
            r(env.package_database()->begin_repositories()),
            r_end(env.package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (r->name() == RepositoryName("virtuals") || r->name() == RepositoryName("installed_virtuals"))
            continue;
        if (env.master_repository() && r->name() == env.master_repository()->name())
            continue;

        write_repository_header(stringify(*spec), stringify(r->name()));

        std::shared_ptr<const CategoryNamePartSet> cat_names(r->category_names());
        for (CategoryNamePartSet::ConstIterator c(cat_names->begin()), c_end(cat_names->end()) ;
                c != c_end ; ++c)
        {
            cerr << xterm_title("Checking " + stringify(*c) + " - adjutrix");

            if (CommandLine::get_instance()->a_category.specified())
                if (CommandLine::get_instance()->a_category.end_args() == std::find(
                            CommandLine::get_instance()->a_category.begin_args(),
                            CommandLine::get_instance()->a_category.end_args(),
                            stringify(*c)))
                    continue;

            std::shared_ptr<const QualifiedPackageNameSet> pkg_names(r->package_names(*c));
            for (QualifiedPackageNameSet::ConstIterator p(pkg_names->begin()), p_end(pkg_names->end()) ;
                    p != p_end ; ++p)
            {
                if (CommandLine::get_instance()->a_package.specified())
                    if (CommandLine::get_instance()->a_package.end_args() == std::find(
                                CommandLine::get_instance()->a_package.begin_args(),
                                CommandLine::get_instance()->a_package.end_args(),
                                stringify(p->package())))
                        continue;

                ret |= check_one_package(env, *entries, *p);
            }
        }
    }

    return ret;
}


