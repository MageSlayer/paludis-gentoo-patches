/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_metadata_key.cc "example_metadata_key.cc" .
 *
 * \ingroup g_metadata_key
 */

/** \example example_metadata_key.cc
 *
 * This example demonstrates how to use MetadataKey. It displays all the
 * metadata keys for a particular PackageID and all repositories.
 */

#include <paludis/paludis.hh>
#include <paludis/util/pretty_print.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <set>
#include <cstdlib>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::left;
using std::setw;

namespace
{
    void show_key(const MetadataKey & key, const std::string & indent = "");

    std::string stringify_string_pair(const std::pair<const std::string, std::string> & s)
    {
        if (s.first.empty())
            return s.second;
        else
            return s.first + "=" + s.second;
    }

    /* We use this visitor to display extra information about a MetadataKey,
     * depending upon its type. */
    class MetadataKeyInformationVisitor
    {
        private:
            /* Because of MetadataSectionKey, we can be called recursively. We add a level
             * of indenting each time. */
            std::string indent;

        public:
            MetadataKeyInformationVisitor(const std::string & i = "") :
                indent(i)
            {
            }

            void visit(const MetadataValueKey<std::string> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataValueKey<std::string>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.parse_value() << endl;
            }

            void visit(const MetadataValueKey<Slot> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataValueKey<SlotName>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.parse_value().raw_value() << endl;
            }

            void visit(const MetadataValueKey<long> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataValueKey<long>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.parse_value() << endl;
            }

            void visit(const MetadataValueKey<bool> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataValueKey<bool>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.parse_value() << endl;
            }

            void visit(const MetadataValueKey<FSPath> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataValueKey<FSPath>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.parse_value() << endl;
            }

            void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " <<
                    "MetadataValueKey<std::shared_ptr<const PackageID> >" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << *key.parse_value() << endl;
            }

            void visit(const MetadataTimeKey & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataTimeKey" << endl;

                cout << indent << left << setw(30) << "    Value:" << " " << pretty_print_time(key.parse_value().seconds()) << endl;
            }

            void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
            {
                cout << indent << left << setw(30) << "    Class:" << " " <<
                    "MetadataValueKey<std::shared_ptr<const Choices> > " << endl;
                /* We won't display the contents of the choices key here, since
                 * it has its own examples. */
            }

            void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<PlainTextSpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<RequiredUseSpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<LicenseSpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<SimpleURISpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<DependencySpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<FetchableURISpecTree>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
                cout << indent << left << setw(30) << "    Initial label:" << " " << key.initial_label()->text() << endl;
            }

            void visit(const MetadataCollectionKey<KeywordNameSet> & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<KeywordNameSet>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << key.pretty_print_value(UnformattedPrettyPrinter(), { }) << endl;
            }

            void visit(const MetadataCollectionKey<Set<std::string> > & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<Set<std::string> >" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << join(value->begin(), value->end(), " ") << endl;
            }

            void visit(const MetadataCollectionKey<Map<std::string, std::string> > & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<Map<std::string, std::string> >" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << join(value->begin(), value->end(), " ", stringify_string_pair) << endl;
            }

            void visit(const MetadataCollectionKey<Sequence<std::string> > & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<Sequence<std::string> >" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << join(value->begin(), value->end(), " ") << endl;
            }

            void visit(const MetadataCollectionKey<Maintainers> & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<Maintainers>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << join(value->begin(), value->end(), " ") << endl;
            }

            void visit(const MetadataCollectionKey<FSPathSequence> & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<FSPathSequence>" << endl;
                cout << indent << left << setw(30) << "    Value:" << " " << join(value->begin(), value->end(), " ") << endl;
            }

            void visit(const MetadataCollectionKey<PackageIDSequence> & key)
            {
                auto value(key.parse_value());
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataCollectionKey<PackageIDSequence>" << endl;
                /* Slight trickery: a PackageIDSequence stores shared pointers
                 * to PackageID instances, so we need indirect_iterator to get
                 * an extra level of dereferencing. */
                cout << indent << left << setw(30) << "    Value:" << " " << join(indirect_iterator(value->begin()),
                        indirect_iterator(value->end()), " ") << endl;
            }

            void visit(const MetadataSectionKey & key)
            {
                cout << indent << left << setw(30) << "    Class:" << " " << "MetadataSectionKey" << endl;
                cout << indent << left << setw(30) << "    Keys:" << endl;

                /* A MetadataSectionKey contains other keys. */
                for (const auto & section_key : key.metadata())
                {
                    show_key(*section_key, indent + "    ");
                    cout << endl;
                }
            }
    };

    /* Display as much as we can about a key. */
    void show_key(const MetadataKey & key, const std::string & indent)
    {
        /* All MetadataKey instances have a raw name, a human readable
         * name and a type. The type is a hint to clients as to whether
         * the key should be displayed when outputting the package (for
         * example, 'paludis --query' shows mkt_significant keys first,
         * then mkt_normal keys, and doesn't show mkt_dependencies
         * without '--show-deps', mkt_author without '--show-author'
         * or mkt_internal without '--show-metadata'. */
        cout << indent << left << setw(30) << "    Raw name:" << " " << key.raw_name() << endl;
        cout << indent << left << setw(30) << "    Human name:" << " " << key.human_name() << endl;
        cout << indent << left << setw(30) << "    Type:" << " " << key.type() << endl;

        /* To get any more information out of a MetadataKey we have to
         * use a visitor. This lets us write type-safe handling code for
         * the appropriate MetadataKey subclass without the need for any
         * runtime type information queries. */
        MetadataKeyInformationVisitor v(indent);
        key.accept(v);
    }
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_metadata_key", "EXAMPLE_METADATA_KEY_OPTIONS", "EXAMPLE_METADATA_KEY_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for 'sys-apps/paludis'. */
        std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsSorted(
                    generator::Package(QualifiedPackageName("sys-apps/paludis")))]);

        /* For each ID: */
        for (const auto & id : *ids)
        {
            cout << *id << ":" << endl;

            /* For each metadata key: */
            for (const auto & key : id->metadata())
            {
                /* Display it. Note that PackageID::MetadataConstIterator returns a std::shared_ptr
                 * to a key, so we dereference it */
                show_key(*key);
                cout << endl;
            }

            cout << endl;
        }

        /* And for each repository: */
        for (const auto & repository : env->repositories())
        {
            cout << repository->name() << ":" << endl;

            /* For each metadata key: */
            for (const auto & key : repository->metadata())
            {
                /* Display it. Repository::MetadataConstIterator also returns a
                 * std::shared_ptr to the key. */
                show_key(*key);
                cout << endl;
            }

            cout << endl;
        }
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

