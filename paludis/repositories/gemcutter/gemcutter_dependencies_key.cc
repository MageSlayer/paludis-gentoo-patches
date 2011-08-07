/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/gemcutter/gemcutter_dependencies_key.hh>
#include <paludis/util/accept_visitor.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/return_literal_function.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/version_requirements.hh>
#include <paludis/version_operator.hh>
#include <paludis/version_spec.hh>
#include <paludis/pretty_printer.hh>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace paludis;
using namespace paludis::gemcutter_repository;

GemcutterDependenciesError::GemcutterDependenciesError(const std::string & s) throw () :
    Exception(s)
{
}

namespace
{
    struct ValuePrinter
    {
        std::stringstream s;
        const PrettyPrinter & printer;
        const PrettyPrintOptions options;

        const unsigned indent;
        const bool flat;
        bool need_space;

        ValuePrinter(
                const PrettyPrinter & p,
                const PrettyPrintOptions & o) :
            printer(p),
            options(o),
            indent(0),
            flat(! options[ppo_multiline_allowed]),
            need_space(false)
        {
        }

        void visit(const DependencySpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (! flat)
                s << printer.indentify(indent + 1);
            else if (need_space)
                s << " ";
            else
                need_space = true;

            s << printer.prettify(*node.spec());

            if (! flat)
                s << printer.newline();
        }

        void visit(const DependencySpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<DependenciesLabelsDepSpec>::Type & node)
        {
            if (! flat)
                s << printer.indentify(indent);
            else if (need_space)
                s << " ";
            else
                need_space = true;

            s << printer.prettify(*node.spec());

            if (! flat)
                s << printer.newline();
        }

        void visit(const DependencySpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }

        void visit(const DependencySpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const DependencySpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };

    struct GemcutterDependenciesKeyData :
        Singleton<GemcutterDependenciesKeyData>
    {
        std::shared_ptr<DependenciesLabelsDepSpec> development_dependencies_label;
        std::shared_ptr<DependenciesLabelsDepSpec> runtime_dependencies_label;
        std::shared_ptr<DependenciesLabelSequence> initial_labels;

        GemcutterDependenciesKeyData() :
            development_dependencies_label(std::make_shared<DependenciesLabelsDepSpec>()),
            runtime_dependencies_label(std::make_shared<DependenciesLabelsDepSpec>()),
            initial_labels(std::make_shared<DependenciesLabelSequence>())
        {
            development_dependencies_label->add_label(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >("development"));
            runtime_dependencies_label->add_label(std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >("runtime"));
            initial_labels->push_back(*development_dependencies_label->begin());
        }
    };

    std::shared_ptr<PackageDepSpec> spec_from(const GemJSONDependency & dep)
    {
        std::vector<std::string> tokens;
        tokenise_whitespace(dep.requirements(), std::back_inserter(tokens));

        if (tokens.size() != 2)
            throw GemcutterDependenciesError("Couldn't parse requirement '" + stringify(dep.requirements()) + "'");

        VersionOperator op(vo_equal);
        if (tokens[0] == "<")
            op = vo_less;
        else if (tokens[0] == "<=")
            op = vo_less_equal;
        else if (tokens[0] == "=")
            op = vo_equal;
        else if (tokens[0] == ">")
            op = vo_greater;
        else if (tokens[0] == ">=")
            op = vo_greater_equal;
        else if (tokens[0] == "~>")
            op = vo_tilde_greater;
        else
            throw GemcutterDependenciesError("Couldn't parse operator in requirement '" + stringify(dep.requirements()) + "'");

        VersionSpec ver(tokens[1], { });

        return make_shared_copy(PackageDepSpec(make_package_dep_spec({ pmpdso_always_use_ranged_deps })
                    .package(QualifiedPackageName("gem/" + dep.name()))
                    .version_requirement(make_named_values<VersionRequirement>(
                            n::version_operator() = op,
                            n::version_spec() = ver
                            ))
                    ));
    }

    std::shared_ptr<DependencySpecTree> build(
            const std::shared_ptr<const GemJSONDependencies> & dd,
            const std::shared_ptr<const GemJSONDependencies> & dr)
    {
        auto result(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>()));

        if (dd)
        {
            result->top()->append(GemcutterDependenciesKeyData::get_instance()->development_dependencies_label);
            for (auto i(dd->dependencies().begin()), i_end(dd->dependencies().end()) ;
                    i != i_end ; ++i)
                result->top()->append(spec_from(*i));
        }

        if (dr)
        {
            result->top()->append(GemcutterDependenciesKeyData::get_instance()->runtime_dependencies_label);
            for (auto i(dr->dependencies().begin()), i_end(dr->dependencies().end()) ;
                    i != i_end ; ++i)
                result->top()->append(spec_from(*i));
        }

        return result;
    }
}

namespace paludis
{
    template <>
    struct Imp<GemcutterDependenciesKey>
    {
        const Environment * const env;
        const std::string * const raw_name;
        const std::string * const human_name;
        const MetadataKeyType type;
        const std::shared_ptr<DependencySpecTree> value;

        Imp(
                const Environment * const e,
                const std::string * const r,
                const std::string * const h,
                const MetadataKeyType t,
                const std::shared_ptr<const GemJSONDependencies> & dd,
                const std::shared_ptr<const GemJSONDependencies> & dr) :
            env(e),
            raw_name(r),
            human_name(h),
            type(t),
            value(build(dd, dr))
        {
        }
    };
}

GemcutterDependenciesKey::GemcutterDependenciesKey(const Environment * const e, const std::string * const r, const std::string * const h,
        const MetadataKeyType t, const std::shared_ptr<const GemJSONDependencies> & dd, const std::shared_ptr<const GemJSONDependencies> & dr) :
    _imp(e, r, h, t, dd, dr)
{
}

GemcutterDependenciesKey::~GemcutterDependenciesKey() = default;

const std::shared_ptr<const DependencySpecTree>
GemcutterDependenciesKey::parse_value() const
{
    return _imp->value;
}

const std::string
GemcutterDependenciesKey::raw_name() const
{
    return *_imp->raw_name;
}

const std::string
GemcutterDependenciesKey::human_name() const
{
    return *_imp->human_name;
}

MetadataKeyType
GemcutterDependenciesKey::type() const
{
    return _imp->type;
}

const std::shared_ptr<const DependenciesLabelSequence>
GemcutterDependenciesKey::initial_labels() const
{
    return GemcutterDependenciesKeyData::get_instance()->initial_labels;
}

const std::string
GemcutterDependenciesKey::pretty_print_value(
        const PrettyPrinter & printer,
        const PrettyPrintOptions & options) const
{
    ValuePrinter p{printer, options};
    _imp->value->top()->accept(p);
    return p.s.str();
}

namespace paludis
{
    template class Pimp<GemcutterDependenciesKey>;
}
