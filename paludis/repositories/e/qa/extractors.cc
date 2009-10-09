/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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

#include "extractors.hh"
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/qa.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <tr1/functional>
#include <algorithm>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct ExtractorsRequirements :
        InstantiationPolicy<ExtractorsRequirements, instantiation_method::SingletonTag>
    {
        std::tr1::shared_ptr<const KeyValueConfigFile> file;

        ExtractorsRequirements(const FSEntry & f = FSEntry(getenv_with_default("PALUDIS_QA_DATA_DIR",
                        stringify(FSEntry(DATADIR) / "paludis" / "qa"))) / "extractors.conf")
        {
            try
            {
                file.reset(new KeyValueConfigFile(f, KeyValueConfigFileOptions(),
                            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation));
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                Log::get_instance()->message("e.metadata_key.extractors.configuration_error", ll_warning, lc_context)
                    << "Got error '" << e.message() << "' (" << e.what()
                    << ") when loading extractors.conf for QA extractors_check";
                file.reset(new KeyValueConfigFile(std::string(), KeyValueConfigFileOptions(),
                            &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation));
            }
        }
    };

    struct FlagExtractor
    {
        std::map<QualifiedPackageName, std::set<ChoiceNameWithPrefix> > relevant;
        std::set<ChoiceNameWithPrefix> current;
        std::set<QualifiedPackageName> needed_packages;

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
        {
            std::string::size_type p(node.spec()->filename().rfind('.'));
            if (std::string::npos == p)
                return;
            std::string extension(node.spec()->filename().substr(p + 1));
            std::string needed(ExtractorsRequirements::get_instance()->file->get(extension));
            if (! needed.empty())
            {
                needed_packages.insert(QualifiedPackageName(needed));
                relevant[QualifiedPackageName(needed)].insert(current.begin(), current.end());
            }
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (node.spec()->package_ptr() && needed_packages.count(*node.spec()->package_ptr()))
                relevant[*node.spec()->package_ptr()].insert(current.begin(), current.end());
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            Save<std::set<ChoiceNameWithPrefix> > save_current(&current);
            current.insert(elike_conditional_dep_spec_flag(*node.spec()));
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }
    };

    struct Requirements
    {
        const QualifiedPackageName & name;
        const std::set<ChoiceNameWithPrefix> & relevant;
        std::map<ChoiceNameWithPrefix, bool> current;
        std::set<std::map<ChoiceNameWithPrefix, bool> > requirements;

        Requirements(const QualifiedPackageName & n, const std::set<ChoiceNameWithPrefix> & r) :
            name(n),
            relevant(r)
        {
        }

        void add_requirements()
        {
            std::set<std::map<ChoiceNameWithPrefix, bool> > new_requirements;
            new_requirements.insert(current);
            for (std::set<ChoiceNameWithPrefix>::const_iterator r(relevant.begin()), r_end(relevant.end()) ;
                    r != r_end ; ++r)
            {
                if (! current.count(*r))
                {
                    std::set<std::map<ChoiceNameWithPrefix, bool> > new_requirements_c;
                    for (std::set<std::map<ChoiceNameWithPrefix, bool> >::iterator i(new_requirements.begin()),
                            i_end(new_requirements.end()) ;
                            i != i_end ; ++i)
                    {
                        std::map<ChoiceNameWithPrefix, bool> n(*i);
                        n[*r] = true;
                        new_requirements_c.insert(n);
                        n[*r] = false;
                        new_requirements_c.insert(n);
                    }
                    std::swap(new_requirements_c, new_requirements);
                }
            }
            requirements.insert(new_requirements.begin(), new_requirements.end());
        }

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type & node)
        {
            std::string::size_type p(node.spec()->filename().rfind('.'));
            if (std::string::npos == p)
                return;
            std::string extension(node.spec()->filename().substr(p + 1));
            std::string needed(ExtractorsRequirements::get_instance()->file->get(extension));
            if (needed.empty() || QualifiedPackageName(needed) != name)
                return;

            add_requirements();
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if ((! node.spec()->package_ptr()) || (name != *node.spec()->package_ptr()))
                return;

            add_requirements();
        }

        void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            Save<std::map<ChoiceNameWithPrefix, bool> > save_current(&current);
            std::pair<std::map<ChoiceNameWithPrefix, bool>::const_iterator, bool> p(current.insert(std::make_pair(
                            elike_conditional_dep_spec_flag(*node.spec()), ! elike_conditional_dep_spec_is_inverse(*node.spec()))));
            if (p.second || (p.first->second == ! elike_conditional_dep_spec_is_inverse(*node.spec())))
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type &)
        {
        }
    };
}

bool
paludis::erepository::extractors_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.extractors_check", ll_debug, lc_context) << "extractors_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (id->fetches_key())
    {
        try
        {
            /* Find all USE flags relevant for our operation. Don't use IUSE,
             * since it needs to work with USE_EXPAND and it's potentially
             * O(2^n). */
            FlagExtractor f;
            id->fetches_key()->value()->root()->accept(f);
            if (id->build_dependencies_key())
                id->build_dependencies_key()->value()->root()->accept(f);

            for (std::map<QualifiedPackageName, std::set<ChoiceNameWithPrefix> >::const_iterator
                    r(f.relevant.begin()), r_end(f.relevant.end()) ;
                    r != r_end ; ++r)
            {
                if (r->second.size() > 10)
                {
                    reporter.message(QAMessage(entry, qaml_maybe, name, "Too many flags to determine whether "
                                "extractor dependency requirement '" + stringify(r->first) + "' is met")
                            .with_associated_id(id)
                            .with_associated_key(id, id->fetches_key()));
                    continue;
                }

                /* Find the set of requirements. */
                Requirements q(r->first, r->second);
                id->fetches_key()->value()->root()->accept(q);

                /* Find the set of met requirements. */
                Requirements m(r->first, r->second);
                if (id->build_dependencies_key())
                    id->build_dependencies_key()->value()->root()->accept(m);

                /* Find the set of unmet requirements */
                std::set<std::map<ChoiceNameWithPrefix, bool> > unmet;
                std::set_difference(q.requirements.begin(), q.requirements.end(),
                        m.requirements.begin(), m.requirements.end(), std::inserter(unmet, unmet.begin()));

                /* Simplify the set of unmet requirements: reduce
                 * { { a => x, b => y }, { a => !x, b => y } } to { { b => y } } */
                bool changed(true);
                while (changed)
                {
                    changed = false;
                    std::set<std::map<ChoiceNameWithPrefix, bool> > new_unmet;
                    for (std::set<std::map<ChoiceNameWithPrefix, bool> >::const_iterator i(unmet.begin()), i_end(unmet.end()) ;
                            i != i_end ; ++i)
                    {
                        std::map<ChoiceNameWithPrefix, bool>::const_iterator j_rem(i->end());
                        for (std::map<ChoiceNameWithPrefix, bool>::const_iterator j(i->begin()), j_end(i->end()) ;
                                j != j_end ; ++j)
                        {
                            std::map<ChoiceNameWithPrefix, bool> n(*i);
                            n[j->first] = !n[j->first];
                            if (unmet.count(n))
                            {
                                j_rem = j;
                                changed = true;
                                break;
                            }
                        }

                        if (j_rem != i->end())
                        {
                            std::map<ChoiceNameWithPrefix, bool> n(*i);
                            n.erase(j_rem->first);
                            new_unmet.insert(n);
                        }
                        else
                            new_unmet.insert(*i);
                    }
                    std::swap(unmet, new_unmet);
                }

                /* Simplify the set of unmet requirements: reduce
                 * { { a => x }, { a => x, b => y } } to { { a => x } } */
                changed = true;
                while (changed)
                {
                    changed = false;
                    for (std::set<std::map<ChoiceNameWithPrefix, bool> >::iterator i(unmet.begin()), i_end(unmet.end()) ;
                            i != i_end && ! changed ; ++i)
                    {
                        for (std::set<std::map<ChoiceNameWithPrefix, bool> >::iterator j(unmet.begin()), j_end(unmet.end()) ;
                                j != j_end && ! changed ; ++j)
                        {
                            if (i == j)
                                continue;

                            std::map<ChoiceNameWithPrefix, bool> delta;
                            std::set_difference(i->begin(), i->end(), j->begin(), j->end(), std::inserter(delta, delta.begin()));
                            if (delta.empty())
                            {
                                unmet.erase(j);
                                changed = true;
                            }
                        }
                    }
                }

                /* Messages */
                if (! unmet.empty())
                {
                    std::string cond;
                    if (! unmet.begin()->empty())
                    {
                        cond.append(" for USE \"");
                        bool need_and(false);
                        for (std::set<std::map<ChoiceNameWithPrefix, bool> >::iterator j(unmet.begin()), j_end(unmet.end()) ;
                                j != j_end ; ++j)
                        {
                            if (need_and)
                                cond.append("\" and \"");
                            need_and = true;

                            bool need_space(false);
                            for (std::map<ChoiceNameWithPrefix, bool>::const_iterator i(j->begin()), i_end(j->end()) ;
                                    i != i_end ; ++i)
                            {
                                if (need_space)
                                    cond.append(" ");
                                if (! i->second)
                                    cond.append("!");
                                cond.append(stringify(i->first));
                                need_space = true;
                            }
                        }
                        cond.append("\"");
                    }

                    reporter.message(QAMessage(entry, qaml_maybe, name, "Extractor '" + stringify(r->first)
                                + "' may be required as a build dependency" + cond)
                            .with_associated_id(id)
                            .with_associated_key(id, id->fetches_key()));
                }
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            reporter.message(QAMessage(entry, qaml_severe, name,
                        "Caught exception '" + stringify(e.message()) + "' ("
                        + stringify(e.what()) + ") when handling key '" + id->fetches_key()->raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, id->fetches_key()));
        }
    }

    return true;
}

