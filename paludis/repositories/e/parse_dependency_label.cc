/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/parse_dependency_label.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/util/log.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/dep_label.hh>
#include <paludis/choice.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/always_enabled_dependency_label.hh>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    bool enabled_if_option(
            const Environment * const env,
            const std::shared_ptr<const PackageID> & id,
            const std::string label,
            const ChoiceNameWithPrefix n)
    {
        auto repo(env->package_database()->fetch_repository(id->repository_name()));
        if (repo->installed_root_key())
            return false;

        if (! id->choices_key())
        {
            Log::get_instance()->message("e.dep_parser.label_enabled.no_choices", ll_warning, lc_context)
                << "ID " << *id << " has no choices, so cannot tell whether label '" << label << "' is enabled";
            return false;
        }

        const std::shared_ptr<const ChoiceValue> v(id->choices_key()->value()->find_by_name_with_prefix(n));
        if (! v)
        {
            Log::get_instance()->message("e.dep_parser.label_enabled.no_choice", ll_warning, lc_context)
                << "ID " << *id << " has no choice named '" << n << "', so cannot tell whether label '"
                << label << "' is enabled";
            return false;
        }

        return v->enabled();
    }

    typedef std::tuple<std::string, std::string, std::string> DepLabelsIndex;

    struct EnabledByChoiceTestDependencyLabel :
        DependenciesTestLabel
    {
        std::string label_text;
        ChoiceNameWithPrefix choice_name;

        EnabledByChoiceTestDependencyLabel(const std::string & s, const ChoiceNameWithPrefix & p) :
            label_text(s),
            choice_name(p)
        {
        }

        virtual bool enabled(const Environment * const env, const std::shared_ptr<const PackageID> & id) const
        {
            return enabled_if_option(env, id, label_text, choice_name);
        }

        virtual const std::string text() const
        {
            return label_text;
        }
    };

    struct DepLabelsStore :
        Singleton<DepLabelsStore>
    {
        Mutex mutex;
        std::map<DepLabelsIndex, std::shared_ptr<DependenciesLabel> > store;

        std::shared_ptr<DependenciesLabel> make(const std::string & class_name, const std::string & text)
        {
            if (class_name == "DependenciesBuildLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesBuildLabelTag> >(text);
            else if (class_name == "DependenciesRunLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRunLabelTag> >(text);
            else if (class_name == "DependenciesPostLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesPostLabelTag> >(text);
            else if (class_name == "DependenciesInstallLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesInstallLabelTag> >(text);
            else if (class_name == "DependenciesCompileAgainstLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesCompileAgainstLabelTag> >(text);
            else if (class_name == "DependenciesFetchLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesFetchLabelTag> >(text);
            else if (class_name == "DependenciesSuggestionLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesSuggestionLabelTag> >(text);
            else if (class_name == "DependenciesRecommendationLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesRecommendationLabelTag> >(text);
            else if (class_name == "DependenciesTestLabel")
                return std::make_shared<AlwaysEnabledDependencyLabel<DependenciesTestLabelTag> >(text);
            else
                throw EDepParseError(text, "Label '" + text + "' maps to unknown class '" + class_name + "'");
        }

        std::shared_ptr<DependenciesLabel> make_test(const std::string & class_name,
                const ChoiceNameWithPrefix & c, const std::string & text)
        {
            if (class_name == "DependenciesTestLabel")
                return std::make_shared<EnabledByChoiceTestDependencyLabel>(text, c);
            else
                throw EDepParseError(text, "Label '" + text + "' maps to unknown test label class '" + class_name + "'");
        }

        std::shared_ptr<DependenciesLabel> get(const std::string & eapi_name, const std::string & class_name, const std::string & text)
        {
            Lock lock(mutex);
            DepLabelsIndex x{eapi_name, class_name, text};

            auto i(store.find(x));
            if (i == store.end())
                i = store.insert(std::make_pair(x, make(class_name, text))).first;
            return i->second;
        }

        std::shared_ptr<DependenciesLabel> get_test(const std::string & eapi_name, const std::string & class_name,
                const ChoiceNameWithPrefix & choice_name, const std::string & text)
        {
            Lock lock(mutex);
            DepLabelsIndex x{eapi_name, class_name, stringify(choice_name) + "/" + text};

            auto i(store.find(x));
            if (i == store.end())
                i = store.insert(std::make_pair(x, make_test(class_name, choice_name, text))).first;
            return i->second;
        }
    };
}

std::shared_ptr<DependenciesLabelsDepSpec>
paludis::erepository::parse_dependency_label(
        const Environment * const,
        const std::string & s,
        const EAPI & e)
{
    Context context("When parsing label string '" + s + "' using EAPI '" + e.name() + "':");

    if (s.empty())
        throw EDepParseError(s, "Empty label");

    std::set<std::string> labels;
    std::string label(s.substr(0, s.length() - 1));
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(label, "+", "", std::inserter(labels, labels.end()));

    std::shared_ptr<DependenciesLabelsDepSpec> l(std::make_shared<DependenciesLabelsDepSpec>());

    for (std::set<std::string>::iterator it = labels.begin(), it_e = labels.end(); it != it_e; ++it)
    {
        std::string c(e.supported()->dependency_labels()->class_for_label(*it)), cc;
        if (c.empty())
            throw EDepParseError(s, "Unknown label '" + *it + "'");

        std::string::size_type p(c.find('/'));
        if (std::string::npos != p)
        {
            cc = c.substr(p + 1);
            c.erase(p);
        }

        if (c == "DependenciesTestLabel")
        {
            if (cc.empty())
                l->add_label(DepLabelsStore::get_instance()->get(e.name(), c, *it));
            else
                l->add_label(DepLabelsStore::get_instance()->get_test(e.name(), c, ChoiceNameWithPrefix(cc), *it));
        }
        else
            l->add_label(DepLabelsStore::get_instance()->get(e.name(), c, *it));
    }

    return l;
}

