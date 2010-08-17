/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include "cmd_show.hh"
#include "colour_formatter.hh"
#include "format_general.hh"
#include "formats.hh"
#include "exceptions.hh"
#include "select_format_for_spec.hh"
#include "format_user_config.hh"
#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/choice.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/mask_utils.hh>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>

#include "command_command_line.hh"

using namespace paludis;
using namespace cave;
using std::cout;
using std::endl;

namespace
{
#include "cmd_show-fmt.hh"

    struct ShowCommandLine :
        CaveCommandCommandLine
    {
        virtual std::string app_name() const
        {
            return "cave show";
        }

        virtual std::string app_synopsis() const
        {
            return "Display a summary of a given object.";
        }

        virtual std::string app_description() const
        {
            return "Displays a formatted summary of a given object. If the object is a set, the set's "
                "contents are listed. If the object is a repository name, information about the repository "
                "is displayed. If the object is a package dep spec with wildcards, possible expansions "
                "are shown. If the object is a package dep spec without wildcards, information about matching "
                "IDs are shown.";
        }

        args::ArgsGroup g_object_options;
        args::EnumArg a_type;

        args::ArgsGroup g_key_options;
        args::SwitchArg a_complex_keys;
        args::SwitchArg a_internal_keys;
        args::SwitchArg a_significant_keys_only;
        args::StringSetArg a_key;

        args::ArgsGroup g_display_options;
        args::SwitchArg a_flat;
        args::SwitchArg a_raw_names;
        args::SwitchArg a_one_version;
        args::SwitchArg a_no_versions;

        ShowCommandLine() :
            g_object_options(main_options_section(), "Object Options", "Alter how objects are interpreted."),
            a_type(&g_object_options, "type", 't', "Specify the type of the specified objects.",
                    args::EnumArg::EnumArgOptions
                    ("auto",               'a', "Automatically determine the type")
                    ("repository",         'r', "Treat the objects as repository names")
                    ("set",                's', "Treat the objects as set names")
                    ("wildcard",           'w', "Treat the objects as a wildcarded package spec")
                    ("package",            'p', "Treat the objects as an unwildcarded package spec, showing all matches for wildcards"),
                    "auto"),
            g_key_options(main_options_section(), "Key Options", "Control which keys are shown."),
            a_complex_keys(&g_key_options, "complex-keys", 'c',
                    "Show complex keys", true),
            a_internal_keys(&g_key_options, "internal-keys", 'i',
                    "Show keys marked as 'internal-only'", true),
            a_significant_keys_only(&g_key_options, "significant-keys-only", 's',
                    "Show only keys marked as 'significant'", true),
            a_key(&g_key_options, "key", 'k',
                    "Show keys with the given name, regardless of other options. May be specified multiple times."),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_flat(&g_display_options, "flat", 'f',
                    "Do not spread key values over multiple lines", true),
            a_raw_names(&g_display_options, "raw-names", 'r',
                    "Display raw rather than human readable key names", true),
            a_one_version(&g_display_options, "one-version", '1',
                    "Display only a single version of any package, rather than all installed and the "
                    "best installable package", true),
            a_no_versions(&g_display_options, "no-versions", '0',
                    "Don't display any version-specific information", true)
        {
            add_usage_line("spec ...");
        }
    };

    std::string slot_as_string(const std::shared_ptr<const PackageID> & id)
    {
        if (id->slot_key())
            return stringify(id->slot_key()->value());
        else
            return "";
    }

    struct SetDisplayer
    {
        const std::shared_ptr<const Environment> env;
        int indent;
        std::set<SetName> recursing_sets;

        SetDisplayer(const std::shared_ptr<const Environment> & e, const int i) :
            env(e),
            indent(i)
        {
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            cout << fuc(select_format_for_spec(env, *node.spec(),
                        fs_set_spec_installed(),
                        fs_set_spec_installable(),
                        fs_set_spec_unavailable()),
                    fv<'s'>(stringify(*node.spec())),
                    fv<'i'>(std::string(indent, ' ')));
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            cout << fuc(fs_set_set(), fv<'s'>(stringify(*node.spec())), fv<'i'>(std::string(indent, ' ')));

            const std::shared_ptr<const SetSpecTree> set(env->set(node.spec()->name()));
            if (! set)
                throw NoSuchSetError(stringify(node.spec()->name()));

            if (! recursing_sets.insert(node.spec()->name()).second)
                throw RecursivelyDefinedSetError(stringify(node.spec()->name()));
            ++indent;

            set->top()->accept(*this);

            recursing_sets.erase(node.spec()->name());
            --indent;
        }

        void visit(const SetSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };

    void do_one_set(const std::shared_ptr<Environment> & env, const SetName & s)
    {
        cout << fuc(fs_set_heading(), fv<'s'>(stringify(s)));

        const std::shared_ptr<const SetSpecTree> set(env->set(s));
        if (! set)
            throw NoSuchSetError(stringify(s));

        SetDisplayer d(env, 1);
        set->top()->accept(d);

        cout << endl;
    }

    void do_one_wildcard(const std::shared_ptr<Environment> & env, const PackageDepSpec & s)
    {
        cout << fuc(fs_wildcard_heading(), fv<'s'>(stringify(s)));

        const std::shared_ptr<const PackageIDSequence> names((*env)[selection::BestVersionOnly(generator::Matches(s, { }))]);
        if (names->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(names->begin()), i_end(names->end()) ;
                i != i_end ; ++i)
        {
            PackageDepSpec name_spec(make_package_dep_spec({ }).package((*i)->name()));
            cout << fuc(select_format_for_spec(env, name_spec,
                        fs_wildcard_spec_installed(),
                        fs_wildcard_spec_installable(),
                        fs_wildcard_spec_unavailable()
                        ),
                    fv<'s'>(stringify(name_spec)));
        }

        cout << endl;
    }

    struct MetadataKeyComparator
    {
        bool operator() (const std::shared_ptr<const MetadataKey> & a, const std::shared_ptr<const MetadataKey> & b) const
        {
            bool a_is_section(simple_visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(simple_visitor_cast<const MetadataSectionKey>(*b));
            if (a_is_section != b_is_section)
                return b_is_section;
            if (a->type() != b->type())
                return a->type() < b->type();
            return a->human_name() < b->human_name();
        }
    };

    struct ContentsDisplayer
    {
        const unsigned indent;
        std::stringstream s;

        ContentsDisplayer(const unsigned i) :
            indent(i)
        {
        }

        void visit(const ContentsFileEntry & e)
        {
            s << fuc(fs_contents_file(), fv<'r'>(stringify(e.location_key()->value())), fv<'b'>(indent ? "true" : ""));
        }

        void visit(const ContentsDirEntry & e)
        {
            s << fuc(fs_contents_dir(), fv<'r'>(stringify(e.location_key()->value())), fv<'b'>(indent ? "true" : ""));
        }

        void visit(const ContentsSymEntry & e)
        {
            s << fuc(fs_contents_sym(), fv<'r'>(stringify(e.location_key()->value())), fv<'b'>(indent ? "true" : ""),
                    fv<'v'>(e.target_key()->value()));
        }

        void visit(const ContentsOtherEntry & e)
        {
            s << fuc(fs_contents_other(), fv<'r'>(stringify(e.location_key()->value())), fv<'b'>(indent ? "true" : ""));
        }
    };

    bool want_key(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<const MetadataKey> & key)
    {
        if (cmdline.a_key.end_args() != std::find(cmdline.a_key.begin_args(), cmdline.a_key.end_args(), key->raw_name()))
            return true;

        if (key->type() == mkt_internal && ! cmdline.a_internal_keys.specified())
            return false;

        if (key->type() != mkt_significant && cmdline.a_significant_keys_only.specified())
            return false;

        return true;
    }

    std::string added_or_changed_string(
            const std::shared_ptr<const Choice> & choice,
            const std::shared_ptr<const ChoiceValue> & value,
            const std::shared_ptr<const PackageID> & maybe_old_id,
            const bool old_id_is_installed)
    {
        std::shared_ptr<const ChoiceValue> maybe_old_value;
        if (maybe_old_id && maybe_old_id->choices_key())
            maybe_old_value = maybe_old_id->choices_key()->value()->find_by_name_with_prefix(value->name_with_prefix());

        if (maybe_old_value)
        {
            if (maybe_old_value->enabled() != value->enabled())
                return "*";
        }
        else if (maybe_old_id && value->explicitly_listed() && choice->consider_added_or_changed())
        {
            if (old_id_is_installed)
                return "+";
            else
                return "-";
        }

        return "";
    }

    struct InfoDisplayer
    {
        const ShowCommandLine & cmdline;
        const int indent;
        const bool important;
        const std::shared_ptr<const PackageID> maybe_old_id;
        const bool old_id_is_installed;

        InfoDisplayer(const ShowCommandLine & c, const int i, const bool m,
                const std::shared_ptr<const PackageID> & o, const bool b) :
            cmdline(c),
            indent(i),
            important(m),
            maybe_old_id(o),
            old_id_is_installed(b)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_subsection_raw() : fs_metadata_subsection_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'i'>(std::string(indent, ' ')));

            std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(k.begin_metadata(), k.end_metadata());
            for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                    s(keys.begin()), s_end(keys.end()) ; s != s_end ; ++s)
            {
                InfoDisplayer i(cmdline, indent + 1, ((*s)->type() == mkt_significant), maybe_old_id, old_id_is_installed);
                if (want_key(cmdline, *s))
                    accept_visitor(i)(**s);
            }
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourFormatter f(indent);
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_flat(f)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_flat(f)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourFormatter f(indent);
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_flat(f)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourFormatter f(indent);
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_flat(f)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(join(k.value()->begin(), k.value()->end(), ", ")),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourFormatter f(indent);
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_flat(f)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(k.pretty_print_flat(f)),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : "")
                        );
            }
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_flat(f)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                else
                {
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_flat(f)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                else
                {
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(k.pretty_print_flat(f)),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : "")
                        );
            }
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourFormatter f(indent);
                if (cmdline.a_flat.specified())
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_flat(f)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                else
                {
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                    cout << k.pretty_print(f);
                }
            }
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<long> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<FSEntry> & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(*k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Contents> > & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                if (cmdline.a_flat.specified())
                {
                    ContentsDisplayer d(0);
                    std::for_each(indirect_iterator(k.value()->begin()),
                            indirect_iterator(k.value()->end()), accept_visitor(d));
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(d.s.str()),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                }
                else
                {
                    ContentsDisplayer d(indent);
                    std::for_each(indirect_iterator(k.value()->begin()),
                            indirect_iterator(k.value()->end()), accept_visitor(d));
                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );
                    cout << d.s.str();
                }
            }
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > & k)
        {
            if (cmdline.a_flat.specified())
            {
                std::stringstream s;
                bool empty_prefix(true);
                for (Choices::ConstIterator c(k.value()->begin()), c_end(k.value()->end()) ;
                        c != c_end ; ++c)
                {
                    if (! cmdline.a_internal_keys.specified())
                    {
                        if ((*c)->hidden())
                            continue;
                        if ((*c)->begin() == (*c)->end())
                            continue;

                        bool any_explicit(false);
                        for (Choice::ConstIterator v((*c)->begin()), v_end((*c)->end()) ;
                                v != v_end ; ++v)
                            if ((*v)->explicitly_listed())
                            {
                                any_explicit = true;
                                break;
                            }

                        if (! any_explicit)
                            continue;
                    }

                    if ((! empty_prefix) || (! (*c)->show_with_no_prefix()))
                    {
                        s << (*c)->prefix() << ": ";
                        empty_prefix = false;
                    }

                    for (Choice::ConstIterator v((*c)->begin()), v_end((*c)->end()) ;
                            v != v_end ; ++v)
                    {
                        if (! cmdline.a_internal_keys.specified())
                            if (! (*v)->explicitly_listed())
                                continue;

                        if ((*v)->enabled())
                        {
                            if ((*v)->locked())
                                s << fuc(fs_choice_forced_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) << " ";
                            else
                                s << fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) << " ";
                        }
                        else
                        {
                            if ((*v)->locked())
                                s << fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) << " ";
                            else
                                s << fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) << " ";
                        }
                    }
                }

                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(s.str()),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : "")
                        );
            }
            else
            {
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(""),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : "")
                        );

                for (Choices::ConstIterator c(k.value()->begin()), c_end(k.value()->end()) ;
                        c != c_end ; ++c)
                {
                    if (! cmdline.a_internal_keys.specified())
                    {
                        if ((*c)->hidden())
                            continue;
                        if ((*c)->begin() == (*c)->end())
                            continue;

                        bool any_explicit(false);
                        for (Choice::ConstIterator v((*c)->begin()), v_end((*c)->end()) ;
                                v != v_end ; ++v)
                            if ((*v)->explicitly_listed())
                            {
                                any_explicit = true;
                                break;
                            }

                        if (! any_explicit)
                            continue;
                    }

                    cout << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent + 1, ' ')),
                            fv<'b'>(important ? "true" : "")
                            );

                    for (Choice::ConstIterator v((*c)->begin()), v_end((*c)->end()) ;
                            v != v_end ; ++v)
                    {
                        if (! cmdline.a_internal_keys.specified())
                            if (! (*v)->explicitly_listed())
                                continue;

                        if ((*v)->enabled())
                        {
                            if ((*v)->locked())
                            {
                                cout << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_forced_enabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_forced_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : "")
                                        );
                            }
                            else
                            {
                                cout << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : "")
                                        );
                            }
                        }
                        else
                        {
                            if ((*v)->locked())
                            {
                                cout << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : "")
                                        );
                            }
                            else
                            {
                                cout << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : "")
                                        );
                            }
                        }
                    }
                }
            }
        }

        void visit(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > & k)
        {
            if (k.value())
            {
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(stringify(k.value()->mask_file())),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : "")
                        );
                for (Sequence<std::string>::ConstIterator i(k.value()->comment()->begin()), i_end(k.value()->comment()->end()) ;
                        i != i_end ; ++i)
                    cout << fuc(fs_metadata_continued_value(),
                            fv<'v'>(*i),
                            fv<'i'>("")
                            );
            }
        }

        void visit(const MetadataTimeKey & k)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(pretty_print_time(k.value().seconds())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : "")
                    );
        }
    };

    struct MaskDisplayer
    {
        const ShowCommandLine & cmdline;
        const int indent;

        MaskDisplayer(const ShowCommandLine & c, const int i) :
            cmdline(c),
            indent(i)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            if (m.unaccepted_key())
            {
                InfoDisplayer i(cmdline, indent, false, make_null_shared_ptr(), false);
                m.unaccepted_key()->accept(i);
            }
            else
            {
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>("Masked"),
                        fv<'v'>("by " + m.description()),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>("")
                        );
            }
        }

        void visit(const UnsupportedMask & m)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : m.description()),
                    fv<'v'>(m.explanation()),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>("")
                    );
        }

        void visit(const AssociationMask & m)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                    fv<'v'>(stringify(*m.associated_package())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>("")
                    );
        }

        void visit(const UserMask & m)
        {
            cout << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                    fv<'v'>(""),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>("")
                    );
        }

        void visit(const RepositoryMask & m)
        {
            if (m.mask_key())
            {
                InfoDisplayer i(cmdline, indent, false, make_null_shared_ptr(), false);
                m.mask_key()->accept(i);
            }
            else
            {
                cout << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                        fv<'v'>(""),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>("")
                        );
            }
        }
    };

    void do_one_repository(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const RepositoryName & s)
    {
        cout << fuc(fs_repository_heading(), fv<'s'>(stringify(s)));

        const std::shared_ptr<const Repository> repo(env->package_database()->fetch_repository(s));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(repo->begin_metadata(), repo->end_metadata());
        for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            InfoDisplayer i(cmdline, 0, ((*k)->type() == mkt_significant), make_null_shared_ptr(), false);
            if (want_key(cmdline, *k))
                accept_visitor(i)(**k);
        }
        cout << endl;
    }

    void do_one_package_id(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> &,
            const std::shared_ptr<const PackageID> & best,
            const std::shared_ptr<const PackageID> & maybe_old_id,
            const bool old_id_is_installed)
    {
        cout << fuc(fs_package_id_heading(), fv<'s'>(stringify(*best)));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(best->begin_metadata(), best->end_metadata());
        for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            InfoDisplayer i(cmdline, 1, ((*k)->type() == mkt_significant), maybe_old_id, old_id_is_installed);
            if (want_key(cmdline, *k))
                accept_visitor(i)(**k);
        }

        if (best->masked())
        {
            cout << fuc(fs_package_id_masks(), fv<'s'>("Masked"));
            MaskDisplayer d(cmdline, 2);
            std::for_each(indirect_iterator(best->begin_masks()), indirect_iterator(best->end_masks()), accept_visitor(d));
        }

        if (best->begin_overridden_masks() != best->end_overridden_masks())
        {
            cout << fuc(fs_package_id_masks_overridden(), fv<'s'>("Overridden Masks"));
            MaskDisplayer d(cmdline, 2);
            for (PackageID::OverriddenMasksConstIterator m(best->begin_overridden_masks()), m_end(best->end_overridden_masks()) ;
                    m != m_end ; ++m)
                (*m)->mask()->accept(d);
        }
    }

    void do_one_package(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PackageDepSpec & s)
    {
        cout << format_general_s(f::show_package_heading(), stringify(s));

        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::AllVersionsGroupedBySlot(
                    generator::Matches(s, { }))]);
        if (ids->empty())
            throw NothingMatching(s);

        std::shared_ptr<const PackageID> best_installable, best_weak_masked_installable, best_masked_installable, best_not_installed;
        std::shared_ptr<PackageIDSequence> all_installed(std::make_shared<PackageIDSequence>());
        std::set<RepositoryName> repos;
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            if ((*i)->repository()->installed_root_key())
                all_installed->push_back(*i);
            else if ((*i)->supports_action(SupportsActionTest<InstallAction>()))
            {
                if ((*i)->masked())
                {
                    if (not_strongly_masked(*i))
                        best_weak_masked_installable = *i;
                    else
                        best_masked_installable = *i;
                }
                else
                    best_installable = *i;
            }
            else
                best_not_installed = *i;

            repos.insert((*i)->repository()->name());
        }

        if (! best_installable)
            best_installable = best_weak_masked_installable;
        if (! best_installable)
            best_installable = best_masked_installable;
        if (! best_installable)
            best_installable = best_not_installed;

        for (std::set<RepositoryName>::const_iterator r(repos.begin()), r_end(repos.end()) ;
                r != r_end ; ++r)
        {
            cout << format_general_s(f::show_package_repository(), stringify(*r));
            std::string slot_name;
            bool need_space(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->repository()->name() != *r)
                    continue;

                if (slot_name != slot_as_string(*i))
                {
                    if (! slot_name.empty())
                        cout << format_general_s(f::show_package_slot(), slot_name);
                    slot_name = slot_as_string(*i);
                }

                if (need_space)
                    cout << " ";
                need_space = true;

                if ((*i)->repository()->installed_root_key())
                    cout << format_general_s(f::show_package_version_installed(), stringify((*i)->canonical_form(idcf_version)));
                else
                {
                    std::string rr;
                    for (PackageID::OverriddenMasksConstIterator m((*i)->begin_overridden_masks()), m_end((*i)->end_overridden_masks()) ;
                            m != m_end ; ++m)
                        rr.append(stringify((*m)->mask()->key()));

                    if (! rr.empty())
                        rr = "(" + rr + ")";

                    if (! (*i)->masked())
                        cout << format_general_sr(f::show_package_version_installable(), stringify((*i)->canonical_form(idcf_version)), rr);
                    else
                    {
                        std::string rs;
                        for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                                m != m_end ; ++m)
                            rs.append(stringify((*m)->key()));
                        rr = rs + rr;
                        cout << format_general_sr(f::show_package_version_unavailable(), stringify((*i)->canonical_form(idcf_version)), rr);
                    }
                }

                if (best_installable && (**i == *best_installable))
                    cout << format_general_s(f::show_package_best(), "");
            }

            if (slot_name.empty())
                cout << format_general_s(f::show_package_no_slot(), slot_name);
            else
                cout << format_general_s(f::show_package_slot(), slot_name);
            cout << endl;
        }

        if (cmdline.a_no_versions.specified())
        {
        }
        else if (cmdline.a_one_version.specified())
        {
            if (best_installable)
                do_one_package_id(cmdline, env, best_installable, all_installed->empty() ? make_null_shared_ptr() : *all_installed->rbegin(), true);
            else if (! all_installed->empty())
                do_one_package_id(cmdline, env, *all_installed->rbegin(), best_installable, false);
        }
        else
        {
            for (PackageIDSequence::ConstIterator i(all_installed->begin()), i_end(all_installed->end()) ;
                    i != i_end ; ++i)
                do_one_package_id(cmdline, env, *i, best_installable, false);
            if (best_installable)
                do_one_package_id(cmdline, env, best_installable, all_installed->empty() ? make_null_shared_ptr() : *all_installed->rbegin(), true);
        }

        cout << endl;
    }

    void do_all_packages(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PackageDepSpec & s)
    {
        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(generator::Matches(s,
                        { }))]);
        if (ids->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            do_one_package(cmdline, env, PartiallyMadePackageDepSpec(s).package((*i)->name()));
    }
}

bool
ShowCommand::important() const
{
    return true;
}

int
ShowCommand::run(
        const std::shared_ptr<Environment> & env,
        const std::shared_ptr<const Sequence<std::string > > & args
        )
{
    ShowCommandLine cmdline;
    cmdline.run(args, "CAVE", "CAVE_SHOW_OPTIONS", "CAVE_SHOW_CMDLINE");

    if (cmdline.a_help.specified())
    {
        cout << cmdline;
        return EXIT_SUCCESS;
    }

    if (cmdline.begin_parameters() == cmdline.end_parameters())
        throw args::DoHelp("show requires at least one parameter");

    for (ShowCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        if (cmdline.a_type.argument() == "set")
            do_one_set(env, SetName(*p));
        else if (cmdline.a_type.argument() == "repository")
            do_one_repository(cmdline, env, RepositoryName(*p));
        else if (cmdline.a_type.argument() == "wildcard")
            do_one_wildcard(env, parse_user_package_dep_spec(
                        *p, env.get(), { updso_allow_wildcards }));
        else if (cmdline.a_type.argument() == "package")
            do_all_packages(cmdline, env, parse_user_package_dep_spec(
                        *p, env.get(), { updso_allow_wildcards }));
        else if (cmdline.a_type.argument() == "auto")
        {
            try
            {
                PackageDepSpec spec(parse_user_package_dep_spec(*p, env.get(), { updso_throw_if_set, updso_allow_wildcards }));
                if ((! spec.package_ptr()))
                    do_one_wildcard(env, spec);
                else
                    do_one_package(cmdline, env, spec);
                continue;
            }
            catch (const GotASetNotAPackageDepSpec &)
            {
                do_one_set(env, SetName(*p));
                continue;
            }
            catch (const NoSuchPackageError &)
            {
                try
                {
                    RepositoryName repo_name(*p);
                    if (env->package_database()->has_repository_named(repo_name))
                    {
                        do_one_repository(cmdline, env, repo_name);
                        continue;
                    }
                }
                catch (const RepositoryNameError &)
                {
                }
            }

            nothing_matching_error(env.get(), *p, filter::All());
        }
        else
            throw args::DoHelp("bad value '" + cmdline.a_type.argument() + "' for --" + cmdline.a_type.long_name());
    }

    return EXIT_SUCCESS;
}

std::shared_ptr<args::ArgsHandler>
ShowCommand::make_doc_cmdline()
{
    return std::make_shared<ShowCommandLine>();
}

