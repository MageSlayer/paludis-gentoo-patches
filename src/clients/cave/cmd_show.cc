/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include "colour_pretty_printer.hh"
#include "colours.hh"
#include "exceptions.hh"
#include "select_format_for_spec.hh"
#include "format_user_config.hh"
#include "parse_spec_with_nice_error.hh"

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/pretty_print.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/join.hh>

#include <paludis/args/args.hh>
#include <paludis/args/do_help.hh>

#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/repository.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/filter.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/mask.hh>
#include <paludis/choice.hh>
#include <paludis/mask_utils.hh>
#include <paludis/permitted_choice_value_parameter_values.hh>
#include <paludis/contents.hh>
#include <paludis/dep_spec_data.hh>

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <set>
#include <limits>

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
        args::SwitchArg a_no_keys;
        args::SwitchArg a_complex_keys;
        args::SwitchArg a_internal_keys;
        args::SwitchArg a_significant_keys_only;
        args::StringSetArg a_key;
        args::SwitchArg a_description_keys;

        args::ArgsGroup g_display_options;
        args::SwitchArg a_flat;
        args::SwitchArg a_raw_names;

        args::ArgsGroup g_version_options;
        args::SwitchArg a_one_version;
        args::SwitchArg a_all_versions;
        args::SwitchArg a_no_versions;
        args::SwitchArg a_repository_at_a_time;

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
            a_no_keys(&g_key_options, "no-keys", 'n',
                    "Do not show any metadata keys", true),
            a_complex_keys(&g_key_options, "complex-keys", 'c',
                    "Show complex keys (e.g. dependencies, downloads)", true),
            a_internal_keys(&g_key_options, "internal-keys", 'i',
                    "Show keys marked as 'internal-only'", true),
            a_significant_keys_only(&g_key_options, "significant-keys-only", 's',
                    "Show only keys marked as 'significant'", true),
            a_key(&g_key_options, "key", 'k',
                    "Show keys with the given name, regardless of other options. May be specified multiple times."),
            a_description_keys(&g_key_options, "description-keys", 'd',
                    "Show description keys, regardless of other options.", true),
            g_display_options(main_options_section(), "Display Options", "Controls the output format."),
            a_flat(&g_display_options, "flat", 'f',
                    "Do not spread key values over multiple lines", true),
            a_raw_names(&g_display_options, "raw-names", 'r',
                    "Display raw rather than human readable key names", true),
            g_version_options(main_options_section(), "Version Options", "Controls for which versions "
                    "detailed information is shown when displaying packages. By default all installed "
                    "versions and the best installable version are shown."),
            a_one_version(&g_version_options, "one-version", '1',
                    "Display only a single version of any package.", true),
            a_all_versions(&g_version_options, "all-versions", 'a',
                    "Display all versions of packages.", true),
            a_no_versions(&g_version_options, "no-versions", '0',
                    "Don't display any version-specific information", true),
            a_repository_at_a_time(&g_version_options, "repository-at-a-time", 'R',
                    "Group versions by repository, and then show details for each individual repository", true)
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
        std::ostream & out;

        SetDisplayer(const std::shared_ptr<const Environment> & e, const int i, std::ostream & o) :
            env(e),
            indent(i),
            out(o)
        {
        }

        void visit(const SetSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            out << fuc(select_format_for_spec(env, *node.spec(), make_null_shared_ptr(),
                        fs_set_spec_installed(),
                        fs_set_spec_installable(),
                        fs_set_spec_unavailable()),
                    fv<'s'>(stringify(*node.spec())),
                    fv<'i'>(std::string(indent, ' ')));
        }

        void visit(const SetSpecTree::NodeType<NamedSetDepSpec>::Type & node)
        {
            out << fuc(fs_set_set(), fv<'s'>(stringify(*node.spec())), fv<'i'>(std::string(indent, ' ')));

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

        SetDisplayer d(env, 1, cout);
        set->top()->accept(d);

        cout << endl;
    }

    void do_one_wildcard(const std::shared_ptr<Environment> & env, const PackageDepSpec & s)
    {
        cout << fuc(fs_wildcard_heading(), fv<'s'>(stringify(s)));

        const std::shared_ptr<const PackageIDSequence> names((*env)[selection::BestVersionOnly(generator::Matches(s, make_null_shared_ptr(), { }))]);
        if (names->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(names->begin()), i_end(names->end()) ;
                i != i_end ; ++i)
        {
            PackageDepSpec name_spec(MutablePackageDepSpecData({ }).require_package((*i)->name()));
            cout << fuc(select_format_for_spec(env, name_spec, make_null_shared_ptr(),
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
            bool a_is_section(visitor_cast<const MetadataSectionKey>(*a));
            bool b_is_section(visitor_cast<const MetadataSectionKey>(*b));
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
            const std::shared_ptr<const MetadataKey> & key,
            const std::shared_ptr<const PackageID> & maybe_id)
    {
        if (cmdline.a_description_keys.specified() && maybe_id)
        {
            if (maybe_id->short_description_key() && maybe_id->short_description_key()->raw_name() == key->raw_name())
                return true;
            if (maybe_id->long_description_key() && maybe_id->long_description_key()->raw_name() == key->raw_name())
                return true;
        }

        if (cmdline.a_key.end_args() != std::find(cmdline.a_key.begin_args(), cmdline.a_key.end_args(), key->raw_name()))
            return true;

        if (key->type() == mkt_internal && ! cmdline.a_internal_keys.specified())
            return false;

        if (key->type() != mkt_significant && cmdline.a_significant_keys_only.specified())
            return false;

        return ! cmdline.a_no_keys.specified();
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

    struct PermittedChoiceValueParameterValuesDisplayer
    {
        std::ostream & out;
        const std::string actual_value;

        void visit(const PermittedChoiceValueParameterIntegerValue & v) const
        {
            std::string range;

            if (v.minimum_allowed_value() != std::numeric_limits<int>::min())
            {
                if (v.maximum_allowed_value() != std::numeric_limits<int>::max())
                    range = "between " + stringify(v.minimum_allowed_value()) + " and " + stringify(v.maximum_allowed_value());
                else
                    range = ">= " + stringify(v.minimum_allowed_value());
            }
            else if (v.maximum_allowed_value() != std::numeric_limits<int>::max())
                range = "<= " + stringify(v.maximum_allowed_value());

            out << fuc(
                    fs_permitted_choice_value_int(),
                    fv<'r'>(range)
                    );
        }

        void visit(const PermittedChoiceValueParameterEnumValue & v) const
        {
            if (! v.allowed_values_and_descriptions()->empty())
            {
                out << fuc(fs_permitted_choice_value_enum_values());
                for (auto a(v.allowed_values_and_descriptions()->begin()), a_end(v.allowed_values_and_descriptions()->end()) ;
                        a != a_end ; ++a)
                    out << fuc(
                            actual_value == a->first ? fs_permitted_choice_value_enum_value_chosen() : fs_permitted_choice_value_enum_value(),
                            fv<'v'>(a->first), fv<'d'>(a->second));
            }
        }
    };

    struct InfoDisplayer
    {
        const std::shared_ptr<const Environment> env;
        const ShowCommandLine & cmdline;
        const PrettyPrintOptions basic_ppos;
        const int indent;
        const bool important;
        const std::shared_ptr<const PackageID> maybe_current_id;
        const std::shared_ptr<const PackageID> maybe_old_id;
        const bool old_id_is_installed;
        std::ostream & out;

        InfoDisplayer(
                const std::shared_ptr<const Environment> & e,
                const ShowCommandLine & c,
                const PrettyPrintOptions & bp,
                const int i, const bool m,
                const std::shared_ptr<const PackageID> & k,
                const std::shared_ptr<const PackageID> & o, const bool b,
                std::ostream & ou) :
            env(e),
            cmdline(c),
            basic_ppos(bp),
            indent(i),
            important(m),
            maybe_current_id(k),
            maybe_old_id(o),
            old_id_is_installed(b),
            out(ou)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_subsection_raw() : fs_metadata_subsection_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'i'>(std::string(indent, ' ')));

            std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(k.begin_metadata(), k.end_metadata());
            for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                    s(keys.begin()), s_end(keys.end()) ; s != s_end ; ++s)
            {
                InfoDisplayer i(env, cmdline, basic_ppos, indent + 1,
                        ((*s)->type() == mkt_significant), maybe_current_id, maybe_old_id, old_id_is_installed, out);
                if (want_key(cmdline, *s, maybe_current_id))
                    accept_visitor(i)(**s);
            }
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataCollectionKey<Set<std::string> > & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataCollectionKey<FSPathSequence> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(join(k.value()->begin(), k.value()->end(), ", ")),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                out << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : ""),
                        fv<'p'>("")
                        );
            }
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                if (cmdline.a_flat.specified())
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                else
                {
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                    out << k.pretty_print_value(printer, basic_ppos + ppo_multiline_allowed);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                if (cmdline.a_flat.specified())
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                else
                {
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                    out << k.pretty_print_value(printer, basic_ppos + ppo_multiline_allowed);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                if (cmdline.a_flat.specified())
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                else
                {
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                    out << k.pretty_print_value(printer, basic_ppos + ppo_multiline_allowed);
                }
            }
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                out << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : ""),
                        fv<'p'>("")
                        );
            }
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            if (cmdline.a_complex_keys.specified() || important)
            {
                ColourPrettyPrinter printer(env.get(), maybe_current_id, indent);
                if (cmdline.a_flat.specified())
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(k.pretty_print_value(printer, basic_ppos)),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                else
                {
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                    out << k.pretty_print_value(printer, basic_ppos + ppo_multiline_allowed);
                }
            }
        }

        void visit(const MetadataValueKey<std::string> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataValueKey<SlotName> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataValueKey<long> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataValueKey<bool> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataValueKey<FSPath> & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(stringify(*k.value())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
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
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(d.s.str()),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                }
                else
                {
                    ContentsDisplayer d(indent);
                    std::for_each(indirect_iterator(k.value()->begin()),
                            indirect_iterator(k.value()->end()), accept_visitor(d));
                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
                            );
                    out << d.s.str();
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
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)));
                            else
                                s << fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)));
                        }
                        else
                        {
                            if ((*v)->locked())
                                s << fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)));
                            else
                                s << fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                        fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)));
                        }

                        if (! (*v)->parameter().empty())
                            s << fuc(fs_choice_parameter(), fv<'v'>((*v)->parameter()));
                        s << " ";
                    }
                }

                out << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(s.str()),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : ""),
                        fv<'p'>("")
                        );
            }
            else
            {
                out << fuc(
                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                        fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                        fv<'v'>(""),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(important ? "true" : ""),
                        fv<'p'>("")
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

                    out << fuc(
                            (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                            fv<'s'>(cmdline.a_raw_names.specified() ? (*c)->raw_name() : (*c)->human_name()),
                            fv<'v'>(""),
                            fv<'i'>(std::string(indent + 1, ' ')),
                            fv<'b'>(important ? "true" : ""),
                            fv<'p'>("")
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
                                out << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_forced_enabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_forced_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : ""),
                                        fv<'p'>((*v)->parameter())
                                        );
                            }
                            else
                            {
                                out << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_enabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : ""),
                                        fv<'p'>((*v)->parameter())
                                        );
                            }
                        }
                        else
                        {
                            if ((*v)->locked())
                            {
                                out << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_forced_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : ""),
                                        fv<'p'>((*v)->parameter())
                                        );
                            }
                            else
                            {
                                out << fuc(
                                        (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                                        fv<'s'>(cmdline.a_raw_names.specified() ?
                                            fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->name_with_prefix())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed))) :
                                            fuc(fs_choice_disabled(), fv<'s'>(stringify((*v)->unprefixed_name())),
                                                fv<'r'>(added_or_changed_string(*c, *v, maybe_old_id, old_id_is_installed)))),
                                        fv<'v'>((*v)->description()),
                                        fv<'i'>(std::string(indent + 2, ' ')),
                                        fv<'b'>(important ? "true" : ""),
                                        fv<'p'>((*v)->parameter())
                                        );
                            }
                        }

                        if ((*v)->permitted_parameter_values())
                        {
                            PermittedChoiceValueParameterValuesDisplayer d{out, (*v)->parameter()};
                            (*v)->permitted_parameter_values()->accept(d);
                        }
                    }
                }
            }
        }

        void visit(const MetadataTimeKey & k)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_metadata_value_raw() : fs_metadata_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? k.raw_name() : k.human_name()),
                    fv<'v'>(pretty_print_time(k.value().seconds())),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(important ? "true" : ""),
                    fv<'p'>("")
                    );
        }
    };

    struct MaskDisplayer
    {
        const std::shared_ptr<const Environment> env;
        const std::shared_ptr<const PackageID> id;
        const ShowCommandLine & cmdline;
        const PrettyPrintOptions basic_ppos;
        const int indent;
        std::ostream & out;

        MaskDisplayer(const std::shared_ptr<const Environment> & e,
                const std::shared_ptr<const PackageID> & d, const ShowCommandLine & c,
                const PrettyPrintOptions & bp,
                const int i, std::ostream & o) :
            env(e),
            id(d),
            cmdline(c),
            basic_ppos(bp),
            indent(i),
            out(o)
        {
        }

        void visit(const UnacceptedMask & m)
        {
            if (! m.unaccepted_key_name().empty())
            {
                InfoDisplayer i(env, cmdline, basic_ppos, indent, false, make_null_shared_ptr(), make_null_shared_ptr(), false, out);
                (*id->find_metadata(m.unaccepted_key_name()))->accept(i);
            }
            else
            {
                out << fuc(
                        (cmdline.a_raw_names.specified() ? fs_mask_value_raw() : fs_mask_value_human()),
                        fv<'s'>("Masked"),
                        fv<'v'>("by " + m.description()),
                        fv<'t'>(""),
                        fv<'i'>(std::string(indent, ' ')),
                        fv<'b'>(""),
                        fv<'p'>("")
                        );
            }
        }

        void visit(const UnsupportedMask & m)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_mask_value_raw() : fs_mask_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : m.description()),
                    fv<'v'>(m.explanation()),
                    fv<'t'>(""),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(""),
                    fv<'p'>("")
                    );
        }

        void visit(const AssociationMask & m)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_mask_value_raw() : fs_mask_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                    fv<'v'>(stringify(m.associated_package_spec())),
                    fv<'t'>(""),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(""),
                    fv<'p'>("")
                    );
        }

        void visit(const UserMask & m)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_mask_value_raw() : fs_mask_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                    fv<'v'>(""),
                    fv<'t'>(m.token()),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(""),
                    fv<'p'>("")
                    );
        }

        void visit(const RepositoryMask & m)
        {
            out << fuc(
                    (cmdline.a_raw_names.specified() ? fs_mask_value_raw() : fs_mask_value_human()),
                    fv<'s'>(cmdline.a_raw_names.specified() ? stringify(m.key()) : "by " + m.description()),
                    fv<'v'>(stringify(m.mask_file())),
                    fv<'t'>(m.token()),
                    fv<'i'>(std::string(indent, ' ')),
                    fv<'b'>(""),
                    fv<'p'>("")
                    );

            if (! m.comment().empty())
                out << fuc(fs_metadata_continued_value(),
                        fv<'v'>(m.comment()),
                        fv<'i'>(std::string(indent, ' '))
                        );
        }
    };

    void do_one_repository(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PrettyPrintOptions & basic_ppos,
            const RepositoryName & s)
    {
        cout << fuc(fs_repository_heading(), fv<'s'>(stringify(s)));

        const std::shared_ptr<const Repository> repo(env->fetch_repository(s));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(repo->begin_metadata(), repo->end_metadata());
        for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            InfoDisplayer i(env, cmdline, basic_ppos, 0, ((*k)->type() == mkt_significant), make_null_shared_ptr(), make_null_shared_ptr(), false, cout);
            if (want_key(cmdline, *k, make_null_shared_ptr()))
                accept_visitor(i)(**k);
        }
        cout << endl;
    }

    void do_one_package_id(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PrettyPrintOptions & basic_ppos,
            const std::shared_ptr<const PackageID> & best,
            const std::shared_ptr<const PackageID> & maybe_old_id,
            const bool old_id_is_installed,
            std::ostream & out)
    {
        out << fuc(fs_package_id_heading(), fv<'s'>(stringify(*best)));
        std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator> keys(best->begin_metadata(), best->end_metadata());
        for (std::set<std::shared_ptr<const MetadataKey>, MetadataKeyComparator>::const_iterator
                k(keys.begin()), k_end(keys.end()) ; k != k_end ; ++k)
        {
            bool explicit_key(cmdline.a_key.end_args() != std::find(cmdline.a_key.begin_args(), cmdline.a_key.end_args(), (*k)->raw_name()));
            InfoDisplayer i(env, cmdline, basic_ppos, 0, ((*k)->type() == mkt_significant) || explicit_key, best, maybe_old_id, old_id_is_installed, out);
            if (want_key(cmdline, *k, best))
                accept_visitor(i)(**k);
        }

        if (best->masked())
        {
            out << fuc(fs_package_id_masks(), fv<'s'>("Masked"));
            MaskDisplayer d(env, best, cmdline, basic_ppos, 2, out);
            std::for_each(indirect_iterator(best->begin_masks()), indirect_iterator(best->end_masks()), accept_visitor(d));
        }

        if (best->begin_overridden_masks() != best->end_overridden_masks())
        {
            out << fuc(fs_package_id_masks_overridden(), fv<'s'>("Overridden Masks"));
            MaskDisplayer d(env, best, cmdline, basic_ppos, 2, out);
            for (PackageID::OverriddenMasksConstIterator m(best->begin_overridden_masks()), m_end(best->end_overridden_masks()) ;
                    m != m_end ; ++m)
                (*m)->mask()->accept(d);
        }
    }

    void do_one_package_with_ids(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PrettyPrintOptions & basic_ppos,
            const PackageDepSpec &,
            const std::shared_ptr<const PackageIDSequence> & ids,
            std::ostream & header_out,
            std::ostream & rest_out
            )
    {
        std::shared_ptr<const PackageID> best_installable, best_weak_masked_installable, best_masked_installable, best_not_installed;
        std::shared_ptr<PackageIDSequence> all_installed(std::make_shared<PackageIDSequence>()), all_not_installed(std::make_shared<PackageIDSequence>());
        std::set<RepositoryName> repos;
        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
        {
            auto repo(env->fetch_repository((*i)->repository_name()));
            if (repo->installed_root_key())
                all_installed->push_back(*i);
            else
            {
                all_not_installed->push_back(*i);

                if ((*i)->supports_action(SupportsActionTest<InstallAction>()))
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
            }

            repos.insert((*i)->repository_name());
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
            header_out << fuc(fs_package_repository(), fv<'s'>(stringify(*r)));
            std::string slot_name;
            bool need_space(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if ((*i)->repository_name() != *r)
                    continue;

                if (slot_name != slot_as_string(*i))
                {
                    if (! slot_name.empty())
                        header_out << fuc(fs_package_slot(), fv<'s'>(slot_name));
                    slot_name = slot_as_string(*i);
                }

                if (need_space)
                    header_out << " ";
                need_space = true;

                auto repo(env->fetch_repository((*i)->repository_name()));
                if (repo->installed_root_key())
                    header_out << fuc(fs_package_version_installed(), fv<'s'>(stringify((*i)->canonical_form(idcf_version))));
                else
                {
                    std::string rr;
                    for (PackageID::OverriddenMasksConstIterator m((*i)->begin_overridden_masks()), m_end((*i)->end_overridden_masks()) ;
                            m != m_end ; ++m)
                        rr.append(stringify((*m)->mask()->key()));

                    if (! rr.empty())
                        rr = "(" + rr + ")";

                    if (! (*i)->masked())
                        header_out << fuc(fs_package_version_installable(), fv<'s'>(stringify((*i)->canonical_form(idcf_version))), fv<'r'>(rr));
                    else
                    {
                        std::string rs;
                        for (PackageID::MasksConstIterator m((*i)->begin_masks()), m_end((*i)->end_masks()) ;
                                m != m_end ; ++m)
                            rs.append(stringify((*m)->key()));
                        rr = rs + rr;
                        header_out << fuc(fs_package_version_unavailable(), fv<'s'>(stringify((*i)->canonical_form(idcf_version))), fv<'r'>(rr));
                    }
                }

                if (best_installable && (**i == *best_installable))
                    header_out << fuc(fs_package_best());
            }

            if (slot_name.empty())
                header_out << fuc(fs_package_no_slot());
            else
                header_out << fuc(fs_package_slot(), fv<'s'>(slot_name));
            header_out << endl;
        }

        if (cmdline.a_no_versions.specified())
        {
        }
        else if (cmdline.a_one_version.specified())
        {
            if (best_installable)
                do_one_package_id(cmdline, env, basic_ppos, best_installable, all_installed->empty() ? make_null_shared_ptr() : *all_installed->rbegin(),
                        true, rest_out);
            else if (! all_installed->empty())
                do_one_package_id(cmdline, env, basic_ppos, *all_installed->rbegin(), best_installable,
                        false, rest_out);
        }
        else if (cmdline.a_all_versions.specified())
        {
            for (PackageIDSequence::ConstIterator i(all_installed->begin()), i_end(all_installed->end()) ;
                    i != i_end ; ++i)
                do_one_package_id(cmdline, env, basic_ppos, *i, best_installable, false, rest_out);

            for (PackageIDSequence::ConstIterator i(all_not_installed->begin()), i_end(all_not_installed->end()) ;
                    i != i_end ; ++i)
                do_one_package_id(cmdline, env, basic_ppos, *i, all_installed->empty() ? make_null_shared_ptr() : *all_installed->rbegin(), true, rest_out);
        }
        else
        {
            for (PackageIDSequence::ConstIterator i(all_installed->begin()), i_end(all_installed->end()) ;
                    i != i_end ; ++i)
                do_one_package_id(cmdline, env, basic_ppos, *i, best_installable, false, rest_out);
            if (best_installable)
                do_one_package_id(cmdline, env, basic_ppos, best_installable, all_installed->empty() ? make_null_shared_ptr() : *all_installed->rbegin(),
                        true, rest_out);
        }
    }

    void do_one_package(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PrettyPrintOptions & basic_ppos,
            const PackageDepSpec & s)
    {
        cout << fuc(fs_package_heading(), fv<'s'>(stringify(s)));

        auto ids((*env)[selection::AllVersionsGroupedBySlot(generator::Matches(s, make_null_shared_ptr(), { }))]);
        if (ids->empty())
            throw NothingMatching(s);

        if (cmdline.a_repository_at_a_time.specified())
        {
            std::set<RepositoryName> repos;
            for (auto i(ids->begin()), i_end(ids->end()) ; i != i_end ; ++i)
                repos.insert((*i)->repository_name());

            std::stringstream rest_out;

            for (auto r(repos.begin()), r_end(repos.end()) ; r != r_end ; ++r)
            {
                auto r_ids((*env)[selection::AllVersionsGroupedBySlot(generator::Matches(
                                MutablePackageDepSpecData(*s.data())
                                .unrequire_in_repository()
                                .require_in_repository(*r), make_null_shared_ptr(), { }))]);
                if (! r_ids->empty())
                    do_one_package_with_ids(cmdline, env, basic_ppos, s, r_ids, cout, rest_out);
            }

            std::copy((std::istreambuf_iterator<char>(rest_out)), std::istreambuf_iterator<char>(),
                    std::ostreambuf_iterator<char>(cout));
        }
        else
            do_one_package_with_ids(cmdline, env, basic_ppos, s, ids, cout, cout);

        cout << endl;
    }

    void do_all_packages(
            const ShowCommandLine & cmdline,
            const std::shared_ptr<Environment> & env,
            const PrettyPrintOptions & basic_ppos,
            const PackageDepSpec & s)
    {
        const std::shared_ptr<const PackageIDSequence> ids((*env)[selection::BestVersionOnly(generator::Matches(s,
                        make_null_shared_ptr(), { }))]);
        if (ids->empty())
            throw NothingMatching(s);

        for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                i != i_end ; ++i)
            do_one_package(cmdline, env, basic_ppos, MutablePackageDepSpecData(*s.data())
                    .unrequire_package()
                    .require_package((*i)->name()));
    }
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

    PrettyPrintOptions basic_ppos;
    if (cmdline.a_internal_keys.specified())
        basic_ppos += ppo_include_special_annotations;

    for (ShowCommandLine::ParametersConstIterator p(cmdline.begin_parameters()), p_end(cmdline.end_parameters()) ;
            p != p_end ; ++p)
    {
        if (cmdline.a_type.argument() == "set")
            do_one_set(env, SetName(*p));
        else if (cmdline.a_type.argument() == "repository")
            do_one_repository(cmdline, env, basic_ppos, RepositoryName(*p));
        else if (cmdline.a_type.argument() == "wildcard")
            do_one_wildcard(env, parse_spec_with_nice_error(*p, env.get(), { updso_allow_wildcards }, filter::All()));
        else if (cmdline.a_type.argument() == "package")
            do_all_packages(cmdline, env, basic_ppos, parse_spec_with_nice_error(*p, env.get(), { updso_allow_wildcards }, filter::All()));
        else if (cmdline.a_type.argument() == "auto")
        {
            try
            {
                PackageDepSpec spec(parse_spec_with_nice_error(*p, env.get(), { updso_throw_if_set, updso_allow_wildcards }, filter::All()));
                if ((! spec.package_name_requirement()))
                    do_one_wildcard(env, spec);
                else
                    do_one_package(cmdline, env, basic_ppos, spec);
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
                    if (env->has_repository_named(repo_name))
                    {
                        do_one_repository(cmdline, env, basic_ppos, repo_name);
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

CommandImportance
ShowCommand::importance() const
{
    return ci_core;
}

