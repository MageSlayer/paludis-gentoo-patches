/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_tree.cc "example_dep_tree.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_tree.cc
 *
 * This example demonstrates how to handle dependency specs. It looks through
 * all installed packages, and picks out any package whose dependencies include
 * 'app-arch/unzip', or whose fetchable files includes any with a '.zip'
 * extension.
 *
 * See \ref example_dep_label.cc "example_dep_label.cc" for labels.
 * See \ref example_dep_spec.cc "example_dep_spec.cc" for specs.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;

/* We use this map to store accumulated results. The first item in the pair
 * is whether we see a dependency, the second whether we see an extension. */
typedef std::map<std::string, std::pair<bool, bool> > ResultsMap;

namespace
{
    /* This visitor handles collection of packages with interesting
     * dependencies. We use the ConstVisitor<>::VisitConstSequence helper mixin
     * for AllDepSpec and AnyDepSpec, rather than explicitly visiting all the
     * children manually. */
    class DependenciesCollector :
        public ConstVisitor<DependencySpecTree>,
        public ConstVisitor<DependencySpecTree>::VisitConstSequence<DependenciesCollector, AllDepSpec>,
        public ConstVisitor<DependencySpecTree>::VisitConstSequence<DependenciesCollector, AnyDepSpec>
    {
        private:
            const tr1::shared_ptr<const Environment> _env;
            const tr1::shared_ptr<const PackageID> _id;
            ResultsMap & _results;

        public:
            DependenciesCollector(
                    const tr1::shared_ptr<const Environment> & e,
                    const tr1::shared_ptr<const PackageID> & i,
                    ResultsMap & r) :
                _env(e),
                _id(i),
                _results(r)
            {
            }

            using ConstVisitor<DependencySpecTree>::VisitConstSequence<DependenciesCollector, AllDepSpec>::visit_sequence;
            using ConstVisitor<DependencySpecTree>::VisitConstSequence<DependenciesCollector, AnyDepSpec>::visit_sequence;

            void visit_sequence(const UseDepSpec & u,
                    DependencySpecTree::ConstSequenceIterator cur,
                    DependencySpecTree::ConstSequenceIterator end)
            {
                /* Was this use flag enabled (or, if we're inverse, disabled)
                 * when we built this package? */
                if (_env->query_use(u.flag(), *_id) ^ u.inverse())
                    std::for_each(cur, end, accept_visitor(*this));
            }

            void visit_leaf(const PackageDepSpec & spec)
            {
                /* spec.package_ptr() may return a zero pointer if it's a
                 * wildcarded dep. */
                if (spec.package_ptr() && *spec.package_ptr() == QualifiedPackageName("app-arch/unzip"))
                    _results[stringify(*_id)].first = true;
            }

            void visit_leaf(const BlockDepSpec &)
            {
                /* Not interested */
            }

            void visit_leaf(const DependencyLabelsDepSpec &)
            {
                /* Not interested */
            }
    };

    /* This visitor handles collection of packages with interesting
     * filenames. Again, we use the ConstVisitor<>::VisitConstSequence helper mixin
     * for AllDepSpec (AnyDepSpec is not allowed in a FetchableURISpecTree). */
    class FileExtensionsCollector :
        public ConstVisitor<FetchableURISpecTree>,
        public ConstVisitor<FetchableURISpecTree>::VisitConstSequence<FileExtensionsCollector, AllDepSpec>
    {
        private:
            const tr1::shared_ptr<const Environment> _env;
            const tr1::shared_ptr<const PackageID> _id;
            ResultsMap & _results;

        public:
            FileExtensionsCollector(
                    const tr1::shared_ptr<const Environment> & e,
                    const tr1::shared_ptr<const PackageID> & i,
                    ResultsMap & r) :
                _env(e),
                _id(i),
                _results(r)
            {
            }

            using ConstVisitor<FetchableURISpecTree>::VisitConstSequence<FileExtensionsCollector, AllDepSpec>::visit_sequence;

            void visit_sequence(const UseDepSpec & u,
                    FetchableURISpecTree::ConstSequenceIterator cur,
                    FetchableURISpecTree::ConstSequenceIterator end)
            {
                /* Was this use flag enabled (or, if we're inverse, disabled)
                 * when we built this package? */
                if (_env->query_use(u.flag(), *_id) ^ u.inverse())
                    std::for_each(cur, end, accept_visitor(*this));
            }

            void visit_leaf(const FetchableURIDepSpec & spec)
            {
                /* We need to be careful not to assume that the filename has
                 * an extension. */
                std::string::size_type p(spec.filename().rfind('.'));
                if ((std::string::npos != p) && (".zip" == spec.filename().substr(p)))
                    _results[stringify(*_id)].second = true;
            }

            void visit_leaf(const URILabelsDepSpec &)
            {
                /* Not interested */
            }
    };
}

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_dep_tree", "EXAMPLE_DEP_TREE_OPTIONS", "EXAMPLE_DEP_TREE_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for all installed packages. */
        tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(
                    query::SupportsAction<InstalledAction>(),
                    qo_order_by_version));

        ResultsMap results;

        /* For each ID: */
        for (PackageIDSet::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* Ignore old-style virtuals. */
            if ((*i)->virtual_for_key())
                continue;

            /* Insert a default result for this ID */
            results[stringify(**i)] = std::make_pair(false, false);

            /* Create a visitor that collects 'app-arch/unzip' dependencies. */
            DependenciesCollector dependencies_collector(env, *i, results);

            /* IDs can potentially have four dependency-related keys. Each of
             * these keys may return a zero pointer. If it doesn't, visit its
             * value with our collector. */
            if ((*i)->build_dependencies_key())
                (*i)->build_dependencies_key()->value()->accept(dependencies_collector);
            if ((*i)->run_dependencies_key())
                (*i)->run_dependencies_key()->value()->accept(dependencies_collector);
            if ((*i)->post_dependencies_key())
                (*i)->post_dependencies_key()->value()->accept(dependencies_collector);
            if ((*i)->suggested_dependencies_key())
                (*i)->suggested_dependencies_key()->value()->accept(dependencies_collector);

            /* Create a visitor that collects '.zip' file extenstions. */
            FileExtensionsCollector extensions_collector(env, *i, results);

            /* Again, we check for a zero pointer and visit otherwise: */
            if ((*i)->src_uri_key())
                (*i)->src_uri_key()->value()->accept(extensions_collector);
        }

        /* Display our results */
        cout << left << setw(60) << "Package" << "| " << left << setw(4) << "Dep" << "| " << "Ext" << endl;
        cout << std::string(60, '-') << "+" << std::string(4, '-') << "+" << std::string(4, '-') << endl;
        for (ResultsMap::const_iterator r(results.begin()), r_end(results.end()) ;
                r != r_end ; ++r)
            cout << left << setw(60) << r->first << "| "
                << left << setw(4) << (r->second.first ? "yes" : "no") << "| "
                << left << setw(4) << (r->second.second ? "yes" : "no") << endl;
        cout << endl;
    }
    catch (const Exception & e)
    {
        /* Paludis exceptions can provide a handy human-readable backtrace and
         * an explanation message. Where possible, these should be displayed. */
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


