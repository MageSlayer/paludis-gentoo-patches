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
 * metadata keys for a particular PackageID.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <set>
#include <time.h>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::left;
using std::setw;

namespace
{
    /* We use this visitor to display extra information about a MetadataKey,
     * depending upon its type. */
    class MetadataKeyInformationVisitor :
        public ConstVisitor<MetadataKeyVisitorTypes>
    {
        private:
            /* Various methods need a formatter. See \ref example_stringify_formatter.cc
             * "example_stringify_formatter.cc" for more details. */
            StringifyFormatter formatter;

        public:
            void visit(const MetadataStringKey & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataStringKey" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.value() << endl;
            }

            void visit(const MetadataFSEntryKey & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataFSEntryKey" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.value() << endl;
            }

            void visit(const MetadataPackageIDKey & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataPackageIDKey" << endl;
                cout << left << setw(30) << "    Value:" << " " << *key.value() << endl;
            }

            void visit(const MetadataTimeKey & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataTimeKey" << endl;

                /* Yay horrible C formatting routines! */
                time_t t(key.value());
                char buf[255];
                if (! strftime(buf, 254, "%c", gmtime(&t)))
                    buf[0] = '\0';
                cout << left << setw(30) << "    Value:" << " " << buf << endl;
            }

            void visit(const MetadataContentsKey &)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataContentsKey" << endl;
                /* We won't display the contents of the contents key here, since
                 * it involves creating another visitor. See \ref
                 * example_contents.cc "example_contents.cc" for that. */
            }

            void visit(const MetadataRepositoryMaskInfoKey & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataRepositoryMaskInfoKey" << endl;

                /* MetadataRepositoryMaskInfoKey::value() can return a zero
                 * pointer. Other keys can't. */
                if (key.value())
                {
                    cout << left << setw(30) << "    Mask file:" << " " << key.value()->mask_file << endl;
                    /* Comment looks best if it's outputted over multiple lines,
                     * as that's how it tends to be stored in package.mask. */
                    cout << left << setw(30) << "    Comment:" << " ";
                    bool first(true);
                    for (Sequence<std::string>::ConstIterator i(key.value()->comment->begin()),
                            i_end(key.value()->comment->end()) ;
                            i != i_end ; ++i)
                    {
                        if (! first)
                            cout << left << setw(30) << "        ..." << " ";
                        cout << *i << endl;
                        first = false;
                    }
                }
            }

            void visit(const MetadataSpecTreeKey<RestrictSpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<RestrictSpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSpecTreeKey<ProvideSpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<ProvideSpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSpecTreeKey<LicenseSpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<LicenseSpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<SimpleURISpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSpecTreeKey<DependencySpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<DependencySpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<FetchableURISpecTree>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
                cout << left << setw(30) << "    Initial label:" << " " << key.initial_label()->text() << endl;
            }

            void visit(const MetadataSetKey<IUseFlagSet> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<IUseFlagSet>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSetKey<KeywordNameSet> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<KeywordNameSet>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSetKey<UseFlagNameSet> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<UseFlagNameSet>" << endl;
                cout << left << setw(30) << "    Value:" << " " << key.pretty_print_flat(formatter) << endl;
            }

            void visit(const MetadataSetKey<Set<std::string> > & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<Set<std::string> >" << endl;
                cout << left << setw(30) << "    Value:" << " " << join(key.value()->begin(), key.value()->end(), " ") << endl;
            }

            void visit(const MetadataSetKey<PackageIDSequence> & key)
            {
                cout << left << setw(30) << "    Class:" << " " << "MetadataSpecTreeKey<PackageIDSequence>" << endl;
                /* Slight trickery: a PackageIDSequence stores shared pointers
                 * to PackageID instances, so we need indirect_iterator to get
                 * an extra level of dereferencing. */
                cout << left << setw(30) << "    Value:" << " " << join(indirect_iterator(key.value()->begin()),
                        indirect_iterator(key.value()->end()), " ") << endl;
            }
    };
}

int main(int argc, char * argv[])
{
    int exit_status(0);

    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_metadata_key", "EXAMPLE_METADATA_KEY_OPTIONS", "EXAMPLE_METADATA_KEY_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* Fetch package IDs for 'sys-apps/paludis'. */
        tr1::shared_ptr<const PackageIDSequence> ids(env->package_database()->query(
                    query::Matches(PackageDepSpec("sys-apps/paludis", pds_pm_permissive)),
                    qo_order_by_version));

        /* For each ID: */
        for (PackageIDSet::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            cout << **i << ":" << endl;

            /* For each metadata key: */
            for (PackageID::MetadataConstIterator k((*i)->begin_metadata()), k_end((*i)->end_metadata()) ;
                    k != k_end ; ++k)
            {
                /* All MetadataKey instances have a raw name, a human readable
                 * name and a type. The type is a hint to clients as to whether
                 * the key should be displayed when outputting the package (for
                 * example, 'paludis --query' shows mkt_significant keys first,
                 * then mkt_normal keys, and doesn't show mkt_dependencies
                 * without '--show-deps' or mkt_internal without
                 * '--show-metadata'. */
                cout << left << setw(30) << "    Raw name:" << " " << (*k)->raw_name() << endl;
                cout << left << setw(30) << "    Human name:" << " " << (*k)->human_name() << endl;
                cout << left << setw(30) << "    Type:" << " " << (*k)->type() << endl;

                /* To get any more information out of a MetadataKey we have to
                 * use a visitor. This lets us write type-safe handling code for
                 * the appropriate MetadataKey subclass without the need for any
                 * runtime type information queries. */
                MetadataKeyInformationVisitor v;
                (*k)->accept(v);

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



