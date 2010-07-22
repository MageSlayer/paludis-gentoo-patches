/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_label.cc "example_dep_label.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_label.cc
 *
 * This example demonstrates how to handle dependency labels. It produces a
 * summary of distfiles for all installed packages, together with a notice of
 * whether that distfile is fetch-restricted.
 *
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

/* We store our results in a map from distfile name to whether it is fetch
 * restricted. */
typedef std::map<std::string, bool> ResultsMap;

namespace
{
    /* This visitor class is used to determine whether a label represents a
     * fetch restriction. */
    class IsLabelRestrictedVisitor
    {
        public:
            bool result;

            IsLabelRestrictedVisitor(const bool initial) :
                result(initial)
            {
            }

            void visit(const URIListedThenMirrorsLabel &)
            {
                result = false;
            }

            void visit(const URIListedOnlyLabel &)
            {
                result = false;
            }

            void visit(const URIMirrorsOnlyLabel &)
            {
                result = false;
            }

            void visit(const URIMirrorsThenListedLabel &)
            {
                result = false;
            }

            void visit(const URILocalMirrorsOnlyLabel &)
            {
                result = true;
            }

            void visit(const URIManualOnlyLabel &)
            {
                result = true;
            }
    };

    /* This visitor class collects src_uri entries and stores the result in
     * a provided map. Label statuses are handled by a stack. When we enter
     * a block (an AllDepSpec or a ConditionalDepSpec), we duplicate the top item
     * of the stack, since labels recurse into subblocks. When we encounter
     * a label, we replace the top item of the stack. */
    class DistfilesCollector
    {
        private:
            ResultsMap & _results;
            std::list<bool> _restricted;

        public:
            DistfilesCollector(ResultsMap & r, const bool initial) :
                _results(r)
            {
                _restricted.push_back(initial);
            }

            void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
            {
                /* When we encounter an AllDepSpec, duplicate the top item of
                 * our restricted stack, and then recurse over all of its
                 * children, and then restore the stack. */
                _restricted.push_back(_restricted.back());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                _restricted.pop_back();
            }

            void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
            {
                /* Always recurse over a ConditionalDepSpec's children. In real world
                 * code, we would more likely check whether condition is met. */
                _restricted.push_back(_restricted.back());
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
                _restricted.pop_back();
            }

            void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
            {
                /* When we encounter a FetchableURIDepSpec, store its distfile name.
                 * We handle 'a -> b' style specs by taking 'b' as the
                 * distfile name. */
                _results.insert(std::make_pair(node.spec()->filename(), _restricted.back()));
            }

            void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type & node)
            {
                /* Find out whether the label represents a fetch restriction.
                 * Change the top item of the stack as appropriate. Although
                 * a URILabelsDepSpec can contain multiple labels, only the last
                 * one is relevant. */
                IsLabelRestrictedVisitor v(_restricted.back());
                std::for_each(indirect_iterator(node.spec()->begin()), indirect_iterator(node.spec()->end()), accept_visitor(v));
                _restricted.back() = v.result;
            }
    };
}

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_dep_label", "EXAMPLE_DEP_LABEL_OPTIONS", "EXAMPLE_DEP_LABEL_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for all installed packages. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsUnsorted(
                    generator::All() |
                    filter::InstalledAtRoot(FSEntry("/")))]);

        /* Store a map from distfile name to whether it is fetch restricted. */
        ResultsMap results;

        /* For each ID: */
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            /* If we don't have a fetches key, skip this package. All PackageID
             * _key() functions can potentially return zero pointers, so checking is
             * essential. */
            if (! (*i)->fetches_key())
                continue;

            /* We need to know whether the default label for this package's src_uri
             * is restricted. */
            IsLabelRestrictedVisitor is_initial_label_restricted(false);
            (*i)->fetches_key()->initial_label()->accept(is_initial_label_restricted);

            /* Create a visitor that will collect distfiles, and do the collecting. */
            DistfilesCollector collector(results, is_initial_label_restricted.result);
            (*i)->fetches_key()->value()->root()->accept(collector);
        }

        /* Display summary of results */
        cout << left << setw(59) << "Distfile Name" << "| " << "Fetch Restricted?" << endl;
        cout << std::string(59, '-') << "+" << std::string(18, '-') << endl;
        for (ResultsMap::const_iterator r(results.begin()), r_end(results.end()) ;
                r != r_end ; ++r)
            cout << left << setw(59) << r->first << "| " << (r->second ? "yes" : "no") << endl;
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


