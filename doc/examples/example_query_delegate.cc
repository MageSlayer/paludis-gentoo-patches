/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_query_delegate.cc "example_query_delegate.cc" .
 *
 * \ingroup g_query
 */

/** \example example_query_delegate.cc
 *
 * This example demonstrates how to implement a new Query classes. For standard
 * Query classes, see \ref example_query.cc "example_query.cc".
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <algorithm>
#include <iterator>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

namespace
{
    /* To implement a custom query, we need two classes. First, the delegate.
     * We only override the QueryDelegate::ids function, since we can't be more
     * helpful at earlier levels. */
    class DescriptionContainsDelegate :
        public QueryDelegate
    {
        private:
            const std::string _pattern;

        public:
            DescriptionContainsDelegate(const std::string & pattern) :
                _pattern(pattern)
            {
            }

            tr1::shared_ptr<PackageIDSequence>
            ids(const Environment & e,
                    tr1::shared_ptr<const RepositoryNameSequence> repos,
                    tr1::shared_ptr<const QualifiedPackageNameSet> pkgs) const
            {
                tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);

                /* We have to iterate over every repository by hand... */
                for (RepositoryNameSequence::ConstIterator r(repos->begin()), r_end(repos->end()) ;
                         r != r_end ; ++r)
                {
                    /* And from each repository, we iterate over packages by
                     * hand... */
                    tr1::shared_ptr<const Repository> repo(e.package_database()->fetch_repository(*r));
                    for (QualifiedPackageNameSet::ConstIterator p(pkgs->begin()), p_end(pkgs->end()) ;
                            p != p_end ; ++p)
                    {
                        /* And finally, IDs by hand... */
                        tr1::shared_ptr<const PackageIDSequence> i(repo->package_ids(*p));
                        for (PackageIDSequence::ConstIterator v(i->begin()), v_end(i->end()) ;
                                v != v_end ; ++v)
                        {
                            /* Does our description contain the pattern? We must
                             * check for a zero pointer. */
                            if ((*v)->short_description_key())
                                if (std::string::npos != (*v)->short_description_key()->value().find(_pattern))
                                    result->push_back(*v);
                        }
                    }
                }

                return result;
            }

            std::string
            as_human_readable_string() const
            {
                return "description contains '" + _pattern + "'";
            }
    };

    /* Then we implement the Query itself. */
    class DescriptionContains :
        public Query
    {
        public:
            DescriptionContains(const std::string & pattern) :
                Query(tr1::shared_ptr<QueryDelegate>(new DescriptionContainsDelegate(pattern)))
            {
            }
    };

    /* Run a particular query, and show its results. */
    void show_query(const tr1::shared_ptr<const Environment> & env, const Query & query)
    {
        /* Queries support a crude form of stringification. */
        cout << query << ":" << endl;

        /* Usually the only thing clients will do with a Query object is pass it
         * to PackageDatabase::query. */
        tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(query, qo_order_by_version));

        /* Show the results */
        if (! ids->empty())
            std::copy(indirect_iterator(ids->begin()), indirect_iterator(ids->end()),
                    std::ostream_iterator<const PackageID>(cout, "\n"));
        cout << endl;
    }
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_action", "EXAMPLE_ACTION_OPTIONS", "EXAMPLE_ACTION_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Make some queries, and display what they give. */
        show_query(env, DescriptionContains("goat"));

        /* We can combine custom queries too. */
        show_query(env,
                query::SupportsAction<InstalledAction>() &
                DescriptionContains("cow"));
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

    return exit_status;
}



