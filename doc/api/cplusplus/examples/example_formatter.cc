/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_formatter.cc "example_formatter.cc" .
 *
 * \ingroup g_formatters
 */

/** \example example_formatter.cc
 *
 * This example demonstrates how to create a formatter. It outputs information
 * about a package's dependencies in HTML.
 *
 * See \ref example_stringify_formatter.cc "example_stringify_formatter.cc" for
 * StringifyFormatter, a premade formatter that uses stringify.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

namespace
{
    /* Utility function that replaces dodgy characters with HTML escapes. */
    std::string escape_html(const std::string & s)
    {
        std::string result;
        for (std::string::const_iterator i(s.begin()), i_end(s.end()) ;
                i != i_end ; ++i)
            switch (*i)
            {
                case '<':
                    result.append("&lt;");
                    break;

                case '>':
                    result.append("&gt;");
                    break;

                case '&':
                    result.append("&amp;");
                    break;

                default:
                    result.append(std::string(1, *i));
            }

        return result;
    }

    /* Utility function that creates an HTML <span> with a colour. */
    std::string span_colour(const std::string & s, const std::string & c)
    {
        return "<span style=\"color: " + c + "\">" + s + "</span>";
    }

    /* This formatter outputs information about dependencies in HTML. We need
     * to implement CanFormat<> for all of the things that can be found in
     * DependencySpecTree::ItemFormatter, as well as CanSpace. */
    class HTMLFormatter :
        public CanSpace,
        public CanFormat<PackageDepSpec>,
        public CanFormat<DependenciesLabelsDepSpec>,
        public CanFormat<ConditionalDepSpec>,
        public CanFormat<NamedSetDepSpec>,
        public CanFormat<BlockDepSpec>
    {
        public:
            /* The second parameter to the format functions has no meaning
             * beyond being used to overload to the appropriate function. */
            std::string format(const PackageDepSpec & s, const format::Plain &) const
            {
                return span_colour(escape_html(stringify(s)), "#666666");
            }

            std::string format(const PackageDepSpec & s, const format::Installed &) const
            {
                return span_colour(escape_html(stringify(s)), "#6666ff");
            }

            std::string format(const PackageDepSpec & s, const format::Installable &) const
            {
                return span_colour(escape_html(stringify(s)), "#66ff66");
            }

            std::string format(const DependenciesLabelsDepSpec & s, const format::Plain &) const
            {
                return span_colour(escape_html(stringify(s)), "#666666");
            }

            std::string format(const ConditionalDepSpec & s, const format::Plain &) const
            {
                return span_colour(escape_html(stringify(s)), "#666666");
            }

            std::string format(const ConditionalDepSpec & s, const format::Enabled &) const
            {
                return span_colour(escape_html(stringify(s)), "#66ff66");
            }

            std::string decorate(const ConditionalDepSpec &, const std::string & s, const format::Added &) const
            {
                return s;
            }

            std::string decorate(const ConditionalDepSpec &, const std::string & s, const format::Changed &) const
            {
                return s;
            }

            std::string format(const ConditionalDepSpec & s, const format::Disabled &) const
            {
                return span_colour(escape_html(stringify(s)), "#ff6666");
            }

            std::string format(const ConditionalDepSpec & s, const format::Forced &) const
            {
                return span_colour(escape_html("(" + stringify(s) + ")"), "#66ff66");
            }

            std::string format(const ConditionalDepSpec & s, const format::Masked &) const
            {
                return span_colour(escape_html("(" + stringify(s) + ")"), "#ff6666");
            }

            std::string format(const NamedSetDepSpec & s, const format::Plain &) const
            {
                return span_colour(escape_html(stringify(s)), "#666666");
            }

            std::string format(const BlockDepSpec & s, const format::Plain &) const
            {
                return span_colour(escape_html(stringify(s)), "#666666");
            }

            std::string newline() const
            {
                return "<br />\n";
            }

            std::string indent(const int i) const
            {
                std::string result;
                for (int x(0) ; x < i ; ++x)
                    result.append("&nbsp; &nbsp; ");
                return result;
            }
    };
}

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_formatter", "EXAMPLE_FORMATTER_OPTIONS", "EXAMPLE_FORMATTER_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::tr1::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for installable 'sys-apps/paludis'. */
        std::tr1::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")) |
                    filter::SupportsAction<InstallAction>())]);

        /* Write nice valid XHTML, because we're good like that. */
        cout << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"" << endl;
        cout << "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
        cout << "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">" << endl;
        cout << "<head><title>Dependencies for sys-apps/paludis</title></head>" << endl;
        cout << "<body>" << endl;

        /* For each ID: */
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            cout << "<h1>" << escape_html(stringify(**i)) << "</h1>" << endl;

            /* Our formatter. It has no saved state, so we can use a single
             * formatter for all of the keys. */
            HTMLFormatter formatter;

            /* We need to check that _key() methods don't return zero. */
            if ((*i)->build_dependencies_key())
            {
                cout << "<h2>" << escape_html((*i)->build_dependencies_key()->human_name()) << "</h2>" << endl;
                cout << "<div style=\"border: 1px solid #999999; background-color: #eeeeee; "
                    "margin-left: 1em; padding: 0.2em 0.5em; \">" << endl;
                cout << (*i)->build_dependencies_key()->pretty_print(formatter);
                cout << endl << "</div>" << endl;
            }

            if ((*i)->run_dependencies_key())
            {
                cout << "<h2>" << escape_html((*i)->run_dependencies_key()->human_name()) << "</h2>" << endl;
                cout << "<div style=\"border: 1px solid #999999; background-color: #eeeeee; "
                    "margin-left: 1em; padding: 0.2em 0.5em; \">" << endl;
                cout << (*i)->run_dependencies_key()->pretty_print(formatter);
                cout << endl << "</div>" << endl;
            }

            if ((*i)->post_dependencies_key())
            {
                cout << "<h2>" << escape_html((*i)->post_dependencies_key()->human_name()) << "</h2>" << endl;
                cout << "<div style=\"border: 1px solid #999999; background-color: #eeeeee; "
                    "margin-left: 1em; padding: 0.2em 0.5em; \">" << endl;
                cout << (*i)->post_dependencies_key()->pretty_print(formatter);
                cout << endl << "</div>" << endl;
            }

            cout << endl;
        }

        cout << "</body>" << endl;
        cout << "</html>" << endl;
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



