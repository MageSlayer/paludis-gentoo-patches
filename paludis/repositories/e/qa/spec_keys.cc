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

#include "spec_keys.hh"
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/qa.hh>
#include <paludis/dep_spec.hh>
#include <paludis/repository.hh>
#include <paludis/choice.hh>
#include <paludis/util/config_file.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/save.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/elike_conditional_dep_spec.hh>
#include <algorithm>
#include <map>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct SpecKeysBlacklist :
        InstantiationPolicy<SpecKeysBlacklist, instantiation_method::SingletonTag>
    {
        Mutex mutex;
        std::map<std::string, const std::tr1::shared_ptr<const QualifiedPackageNameSet> > map;

        const std::tr1::shared_ptr<const QualifiedPackageNameSet> blacklist(const std::string & s)
        {
            Lock lock(mutex);
            std::map<std::string, const std::tr1::shared_ptr<const QualifiedPackageNameSet> >::const_iterator i(map.find(s));
            if (map.end() != i)
                return i->second;
            else
            {
                Context context("When loading spec_keys PackageDepSpec blacklist '" + s + "':");

                std::tr1::shared_ptr<QualifiedPackageNameSet> r(new QualifiedPackageNameSet);
                FSEntry f(FSEntry(getenv_with_default("PALUDIS_QA_DATA_DIR",
                                stringify(FSEntry(DATADIR) / "paludis" / "qa")))
                        / ("spec_keys_pds_blacklist." + s + ".conf"));

                if (f.exists())
                {
                    LineConfigFile ff(f, LineConfigFileOptions());
                    std::copy(ff.begin(), ff.end(), create_inserter<QualifiedPackageName>(r->inserter()));
                }
                else
                    Log::get_instance()->message("e.qa.spec_keys_check.configuration_error", ll_warning, lc_context)
                        << "Blacklist data file '" << f << "' does not exist";

                map.insert(std::make_pair(s, r));
                return r;
            }
        }
    };

    struct Checker
    {
        const FSEntry entry;
        QAReporter & reporter;
        const std::tr1::shared_ptr<const PackageID> & id;
        const std::tr1::shared_ptr<const MetadataKey> & key;
        const std::string name;
        const std::tr1::shared_ptr<const QualifiedPackageNameSet> pds_blacklist;
        bool forbid_arch_flags, forbid_inverse_arch_flags;

        unsigned level;
        bool child_of_any;
        std::set<ChoiceNameWithPrefix> uses;

        Checker(
                const FSEntry & f,
                QAReporter & r,
                const std::tr1::shared_ptr<const PackageID> & i,
                const std::tr1::shared_ptr<const MetadataKey> & k,
                const std::string & n,
                const std::tr1::shared_ptr<const QualifiedPackageNameSet> p,
                bool a, bool ia) :
            entry(f),
            reporter(r),
            id(i),
            key(k),
            name(n),
            pds_blacklist(p),
            forbid_arch_flags(a),
            forbid_inverse_arch_flags(ia),
            level(0),
            child_of_any(false)
        {
        }

        void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type & node)
        {
            if (pds_blacklist && node.spec()->package_ptr())
            {
                if (pds_blacklist->end() != pds_blacklist->find(*node.spec()->package_ptr()))
                    reporter.message(QAMessage(entry, qaml_maybe, name, "Package '" + stringify(*node.spec())
                                + "' blacklisted in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type & node)
        {
            if (child_of_any)
                reporter.message(QAMessage(entry, qaml_normal, name, "'|| ( )' with block child '"
                            + stringify(*node.spec()) + "' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
        }

        void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &)
        {
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

        void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &)
        {
        }

        void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            if (child_of_any)
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "'|| ( )' with 'use? ( )' child in '"
                            + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));

            if (uses.count(elike_conditional_dep_spec_flag(*node.spec())))
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Recursive use of flag '" + stringify(elike_conditional_dep_spec_flag(*node.spec())) + "' in '"
                            + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));

            std::tr1::shared_ptr<const ChoiceValue> value;
            std::tr1::shared_ptr<const Choice> choice;
            {
                ChoiceNameWithPrefix f(elike_conditional_dep_spec_flag(*node.spec()));
                for (Choices::ConstIterator c(id->choices_key()->value()->begin()), c_end(id->choices_key()->value()->end()) ;
                        c != c_end && ! value ; ++c)
                    for (Choice::ConstIterator i((*c)->begin()), i_end((*c)->end()) ;
                            i != i_end && ! value ; ++i)
                    {
                        if ((*i)->name_with_prefix() == f)
                        {
                            value = *i;
                            choice = *c;
                        }
                    }
            }

            if (! choice)
            {
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Conditional flag '" + stringify(elike_conditional_dep_spec_flag(*node.spec())) +
                            "' in '" + stringify(key->raw_name()) + "' does not exist")
                        .with_associated_id(id)
                        .with_associated_key(id, key)
                        );
            }
            else if (choice->raw_name() == "ARCH")
            {
                if (forbid_arch_flags)
                    reporter.message(QAMessage(entry, qaml_normal, name,
                                "Arch flag '" + stringify(elike_conditional_dep_spec_flag(*node.spec())) + "' in '" + stringify(key->raw_name()) + "'")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
                else if (elike_conditional_dep_spec_is_inverse(*node.spec()) && forbid_inverse_arch_flags)
                    reporter.message(QAMessage(entry, qaml_maybe, name,
                                "Inverse arch flag '" + stringify(elike_conditional_dep_spec_flag(*node.spec())) + "' in '" + stringify(key->raw_name()) + "'")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
            }

            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, false);
            Save<std::set<ChoiceNameWithPrefix> > save_uses(&uses, uses);
            uses.insert(elike_conditional_dep_spec_flag(*node.spec()));
            if (node.begin() == node.end())
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Empty 'use? ( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            else
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type & node)
        {
            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, false);
            if (node.begin() == node.end())
            {
                if (level > 1)
                    reporter.message(QAMessage(entry, qaml_normal, name,
                                "Empty '( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type & node)
        {
            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, true);
            if (node.begin() == node.end())
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Empty '|| ( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            else if (next(node.begin()) == node.end())
            {
                (*node.begin())->accept(*this);
                reporter.message(QAMessage(entry, qaml_normal, name,
                        "'|| ( )' with only one child in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
                std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }
    };

    struct CheckForwarder
    {
        const FSEntry entry;
        QAReporter & reporter;
        std::tr1::shared_ptr<const MetadataKey> key;
        const std::tr1::shared_ptr<const PackageID> & id;
        const std::string name;

        CheckForwarder(
                const FSEntry & f,
                QAReporter & r,
                const std::tr1::shared_ptr<const PackageID> & i,
                const std::string & n) :
            entry(f),
            reporter(r),
            id(i),
            name(n)
        {
        }

        void visit_sptr(const std::tr1::shared_ptr<const MetadataKey> & k)
        {
            key = k;
            k->accept(*this);
        }

        void visit(const MetadataValueKey<std::string> &)
        {
        }

        void visit(const MetadataValueKey<SlotName> &)
        {
        }

        void visit(const MetadataValueKey<long> &)
        {
        }

        void visit(const MetadataValueKey<bool> &)
        {
        }

        void visit(const MetadataTimeKey &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Contents> > &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const PackageID> > &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const RepositoryMaskInfo> > &)
        {
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> &)
        {
        }

        void visit(const MetadataCollectionKey<Set<std::string> > &)
        {
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > &)
        {
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> &)
        {
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> &)
        {
        }

        void visit(const MetadataValueKey<FSEntry> &)
        {
        }

        void visit(const MetadataValueKey<std::tr1::shared_ptr<const Choices> > &)
        {
        }

        void visit(const MetadataSectionKey & k)
        {
            std::for_each(indirect_iterator(k.begin_metadata()),
                    indirect_iterator(k.end_metadata()), accept_visitor(*this));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, std::tr1::shared_ptr<const QualifiedPackageNameSet>(), false, false);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, std::tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, std::tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, SpecKeysBlacklist::get_instance()->blacklist(k.raw_name()), false, true);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, SpecKeysBlacklist::get_instance()->blacklist(k.raw_name()), true, true);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, key, name, std::tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->root()->accept(c);
            }
            catch (const InternalError &)
            {
                throw;
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }
    };
}

bool
paludis::erepository::spec_keys_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const std::tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using spec_keys_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message("e.qa.spec_keys_check", ll_debug, lc_context) << "spec_keys_check '"
        << entry << "', " << *id << "', " << name << "'";

    using namespace std::tr1::placeholders;

    CheckForwarder f(entry, reporter, id, name);
    std::for_each(id->begin_metadata(), id->end_metadata(), std::tr1::bind(&CheckForwarder::visit_sptr, &f, _1));

    return true;
}

