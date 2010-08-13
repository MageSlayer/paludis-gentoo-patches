/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_tag.cc "example_dep_tag.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_tag.cc
 *
 * This example demonstrates how to handle dependency tags. It displays
 * information about the 'security' and 'world' sets.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <set>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;
using std::boolalpha;

/* We store a set of dep tag categories that we've seen. */
typedef std::set<std::string> SeenCategories;

namespace
{
    /* This visitor is used to display information about a tag. */
    class TagDisplayer
    {
        public:
            void visit(const DependencyDepTag & tag)
            {
                /* A DependencyDepTag is used during dependency resolution. It
                 * shows why a package is included on a DepList. It has three
                 * additional fields: an optional package ID that pulled in the
                 * entry, an optional dep spec that pulled in the dependency and
                 * an optional set of conditions upon which that dependency is
                 * active. The third field is not used here, since it is too
                 * complicated for this example. */
                if (tag.package_id())
                    cout << left << setw(20) << "        Package ID:" << " " << *tag.package_id() << endl;
                if (tag.dependency())
                    cout << left << setw(20) << "        Dependency:" << " " << *tag.dependency() << endl;
            }

            void visit(const GLSADepTag & tag)
            {
                /* A GLSADepTag is for security advisories. It carries one
                 * additional field, the GLSA's title. */
                cout << left << setw(20) << "        GLSA title:" << " " << tag.glsa_title() << endl;
            }

            void visit(const GeneralSetDepTag & tag)
            {
                /* A GeneralSetDepTag is for general package sets. It carries
                 * one additional field, the source (e.g. a repository or
                 * environment name). */
                cout << left << setw(20) << "        Source:" << " " << tag.source() << endl;
            }

            void visit(const TargetDepTag &)
            {
                /* A TargetDepTag is used to indicate explicit targets when
                 * resolving dependencies. It carries no extra information. */
            }
    };

    /* Display information about a named set. */
    void display_set(
            const std::shared_ptr<const Environment> & env,
            const SetName & name,
            SeenCategories & seen_categories)
    {
        std::shared_ptr<const SetSpecTree> set(env->set(name));

        /* Environment::set can return a zero pointer, if a set is not known. */
        if (! set)
            return;

        /* The set isn't necessarily flat. We use DepSpecFlattener to make it
         * so, rather than writing a full visitor ourselves. */
        DepSpecFlattener<SetSpecTree, PackageDepSpec> set_flat(env.get());
        set->top()->accept(set_flat);

        cout << "Set '" << name << "':" << endl;

        /* For each item... */
        for (DepSpecFlattener<SetSpecTree, PackageDepSpec>::ConstIterator s(set_flat.begin()),
                s_end(set_flat.end()) ; s != s_end ; ++s)
        {
            /* Ignore it, if it has no tag. */
            if (! (*s)->tag())
                continue;

            cout << "    " << **s << ": " << endl;

            /* All dep tags have short text and a category. As well as
             * displaying the category, we remember it for the categories
             * summary later on. */
            cout << left << setw(20) << "        Short text:" << " " << (*s)->tag()->short_text() << endl;
            cout << left << setw(20) << "        Category:" << " " << (*s)->tag()->category() << endl;
            seen_categories.insert((*s)->tag()->category());

            /* We use a visitor to do extra displaying, so that we can display
             * more detailed information for whatever our tag type is. */
            TagDisplayer displayer;
            (*s)->tag()->accept(displayer);

            cout << endl;
        }

        cout << endl;
    }
}

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_dep_tag", "EXAMPLE_DEP_TAG_OPTIONS", "EXAMPLE_DEP_TAG_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        std::shared_ptr<Environment> env(EnvironmentFactory::get_instance()->create(
                    CommandLine::get_instance()->a_environment.argument()));

        SeenCategories seen_categories;

        /* First, tell us about the 'security' set: */
        display_set(env, SetName("security"), seen_categories);

        /* Then the 'world' set: */
        display_set(env, SetName("world"), seen_categories);

        /* Now display a summary of seen categories. */
        cout << "Seen categories:" << endl;
        for (SeenCategories::const_iterator s(seen_categories.begin()), s_end(seen_categories.end()) ;
                s != s_end ; ++s)
        {
            cout << "    " << *s << ":" << endl;

            /* Fetch the category. */
            std::shared_ptr<const DepTagCategory> category(DepTagCategoryFactory::get_instance()->create(*s));

            cout << left << setw(20) << "        Visible:" << " " << boolalpha << category->visible() << endl;
            cout << left << setw(20) << "        ID:" << " " << category->id() << endl;
            cout << left << setw(20) << "        Title:" << " " << category->title() << endl;
            cout << left << setw(20) << "        Pre text:" << " " << category->pre_text() << endl;
            cout << left << setw(20) << "        Post text:" << " " << category->post_text() << endl;

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

    return EXIT_SUCCESS;
}



