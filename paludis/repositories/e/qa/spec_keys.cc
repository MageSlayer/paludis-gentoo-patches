/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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
#include <paludis/util/config_file.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/save.hh>
#include <paludis/util/set.hh>
#include <paludis/util/system.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator_funcs.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/options.hh>
#include <paludis/util/log.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/create_iterator-impl.hh>
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
        std::map<std::string, const tr1::shared_ptr<const QualifiedPackageNameSet> > map;

        const tr1::shared_ptr<const QualifiedPackageNameSet> blacklist(const std::string & s)
        {
            Lock lock(mutex);
            std::map<std::string, const tr1::shared_ptr<const QualifiedPackageNameSet> >::const_iterator i(map.find(s));
            if (map.end() != i)
                return i->second;
            else
            {
                Context context("When loading spec_keys PackageDepSpec blacklist '" + s + "':");

                tr1::shared_ptr<QualifiedPackageNameSet> r(new QualifiedPackageNameSet);
                FSEntry f(FSEntry(getenv_with_default("PALUDIS_QA_DATA_DIR",
                                stringify(FSEntry(DATADIR) / "paludis" / "qa")))
                        / ("spec_keys_pds_blacklist." + s + ".conf"));

                if (f.exists())
                {
                    LineConfigFile ff(f, LineConfigFileOptions());
                    std::copy(ff.begin(), ff.end(), create_inserter<QualifiedPackageName>(r->inserter()));
                }
                else
                    Log::get_instance()->message(ll_warning, lc_context) << "Blacklist data file '" << f << "' does not exist";

                map.insert(std::make_pair(s, r));
                return r;
            }
        }
    };

    struct Checker :
        ConstVisitor<GenericSpecTree>
    {
        const FSEntry entry;
        QAReporter & reporter;
        const tr1::shared_ptr<const PackageID> & id;
        const std::set<UseFlagName> & iuse_flags;
        const tr1::shared_ptr<const MetadataKey> & key;
        const std::string name;
        const tr1::shared_ptr<const QualifiedPackageNameSet> pds_blacklist;
        bool forbid_arch_flags, forbid_inverse_arch_flags;

        unsigned level;
        bool child_of_any;
        std::set<UseFlagName> uses;

        Checker(
                const FSEntry & f,
                QAReporter & r,
                const tr1::shared_ptr<const PackageID> & i,
                const std::set<UseFlagName> & iuse,
                const tr1::shared_ptr<const MetadataKey> & k,
                const std::string & n,
                const tr1::shared_ptr<const QualifiedPackageNameSet> p,
                bool a, bool ia) :
            entry(f),
            reporter(r),
            id(i),
            iuse_flags(iuse),
            key(k),
            name(n),
            pds_blacklist(p),
            forbid_arch_flags(a),
            forbid_inverse_arch_flags(ia),
            level(0),
            child_of_any(false)
        {
        }

        void visit_leaf(const PackageDepSpec & p)
        {
            if (pds_blacklist && p.package_ptr())
            {
                if (pds_blacklist->end() != pds_blacklist->find(*p.package_ptr()))
                    reporter.message(QAMessage(entry, qaml_maybe, name, "Package '" + stringify(p)
                                + "' blacklisted in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit_leaf(const BlockDepSpec & b)
        {
            if (child_of_any)
                reporter.message(QAMessage(entry, qaml_normal, name, "'|| ( )' with block child '!"
                            + stringify(*b.blocked_spec()) + "' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
        }

        void visit_leaf(const SimpleURIDepSpec &)
        {
        }

        void visit_leaf(const FetchableURIDepSpec &)
        {
        }

        void visit_leaf(const PlainTextDepSpec &)
        {
        }

        void visit_leaf(const URILabelsDepSpec &)
        {
        }

        void visit_leaf(const DependencyLabelsDepSpec &)
        {
        }

        void visit_leaf(const LicenseDepSpec &)
        {
        }

        void visit_leaf(const NamedSetDepSpec &)
        {
        }

        void visit_sequence(const UseDepSpec & u,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            if (child_of_any)
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "'|| ( )' with 'use? ( )' child in '"
                            + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));

            if (uses.count(u.flag()))
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Recursive use of flag '" + stringify(u.flag()) + "' in '"
                            + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));

            if (id->repository()->use_interface->arch_flags()->count(u.flag()))
            {
                if (forbid_arch_flags)
                    reporter.message(QAMessage(entry, qaml_normal, name,
                                "Arch flag '" + stringify(u.flag()) + "' in '" + stringify(key->raw_name()) + "'")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
                else if (u.inverse() && forbid_inverse_arch_flags)
                    reporter.message(QAMessage(entry, qaml_maybe, name,
                                "Inverse arch flag '" + stringify(u.flag()) + "' in '" + stringify(key->raw_name()) + "'")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
            }

            else
            {
                if (iuse_flags.end() == iuse_flags.find(u.flag()))
                {
                    std::tr1::shared_ptr<const UseFlagNameSet> c(
                        id->repository()->use_interface->use_expand_hidden_prefixes());
                    std::string flag(stringify(u.flag()));
                    bool is_hidden(false);

                    for (UseFlagNameSet::ConstIterator i(c->begin()), i_end(c->end()) ;
                            i != i_end ; ++i)
                    {
                        std::string prefix(stringify(*i) + id->repository()->use_interface->use_expand_separator(*id));
                        if (0 == flag.compare(0, prefix.length(), prefix))
                        {
                            is_hidden = true;
                            break;
                        }
                    }

                    if (! is_hidden)
                        reporter.message(QAMessage(entry, qaml_normal, name,
                                    "Conditional flag '" + stringify(u.flag()) +
                                    "' in '" + stringify(key->raw_name()) + "' not in '" +
                                    stringify(id->iuse_key()->raw_name()) + "'")
                                    .with_associated_id(id)
                                    .with_associated_key(id, key)
                                    .with_associated_key(id, id->iuse_key()));
                }
            }

            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, false);
            Save<std::set<UseFlagName> > save_uses(&uses, uses);
            uses.insert(u.flag());
            if (cur == end)
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Empty 'use? ( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            else
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AllDepSpec &,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, false);
            if (cur == end)
            {
                if (level > 1)
                    reporter.message(QAMessage(entry, qaml_normal, name,
                                "Empty '( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
                std::for_each(cur, end, accept_visitor(*this));
        }

        void visit_sequence(const AnyDepSpec &,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, true);
            if (cur == end)
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Empty '|| ( )' in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            else if (next(cur) == end)
            {
                cur->accept(*this);
                reporter.message(QAMessage(entry, qaml_normal, name,
                        "'|| ( )' with only one child in '" + stringify(key->raw_name()) + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
            else
                std::for_each(cur, end, accept_visitor(*this));
        }
    };

    struct CheckForwarder :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        const FSEntry entry;
        QAReporter & reporter;
        tr1::shared_ptr<const MetadataKey> key;
        const tr1::shared_ptr<const PackageID> & id;
        const std::set<UseFlagName> & iuse_flags;
        const std::string name;

        CheckForwarder(
                const FSEntry & f,
                QAReporter & r,
                const tr1::shared_ptr<const PackageID> & i,
                const std::set<UseFlagName> & iuse,
                const std::string & n) :
            entry(f),
            reporter(r),
            id(i),
            iuse_flags(iuse),
            name(n)
        {
        }

        void visit_sptr(const tr1::shared_ptr<const MetadataKey> & k)
        {
            key = k;
            k->accept(*this);
        }

        void visit(const MetadataStringKey &)
        {
        }

        void visit(const MetadataSizeKey &)
        {
        }

        void visit(const MetadataTimeKey &)
        {
        }

        void visit(const MetadataContentsKey &)
        {
        }

        void visit(const MetadataPackageIDKey &)
        {
        }

        void visit(const MetadataRepositoryMaskInfoKey &)
        {
        }

        void visit(const MetadataCollectionKey<IUseFlagSet> &)
        {
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> &)
        {
        }

        void visit(const MetadataCollectionKey<Set<std::string> > &)
        {
        }

        void visit(const MetadataCollectionKey<FSEntrySequence> &)
        {
        }

        void visit(const MetadataCollectionKey<UseFlagNameSet> &)
        {
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> &)
        {
        }

        void visit(const MetadataFSEntryKey &)
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
                Checker c(entry, reporter, id, iuse_flags, key, name, tr1::shared_ptr<const QualifiedPackageNameSet>(), false, false);
                k.value()->accept(c);
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
                Checker c(entry, reporter, id, iuse_flags, key, name, tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->accept(c);
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
                Checker c(entry, reporter, id, iuse_flags, key, name, tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->accept(c);
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
                Checker c(entry, reporter, id, iuse_flags, key, name, SpecKeysBlacklist::get_instance()->blacklist(k.raw_name()), false, true);
                k.value()->accept(c);
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
                Checker c(entry, reporter, id, iuse_flags, key, name, SpecKeysBlacklist::get_instance()->blacklist(k.raw_name()), true, true);
                k.value()->accept(c);
            }
            catch (const Exception & e)
            {
                reporter.message(QAMessage(entry, qaml_severe, name, "Caught exception '" + stringify(e.message()) + "' ("
                            + stringify(e.what()) + ") when handling key '" + k.raw_name() + "'")
                            .with_associated_id(id)
                            .with_associated_key(id, key));
            }
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
        {
            try
            {
                Context context("When visiting metadata key '" + k.raw_name() + "':");
                Checker c(entry, reporter, id, iuse_flags, key, name, tr1::shared_ptr<const QualifiedPackageNameSet>(), true, true);
                k.value()->accept(c);
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
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using spec_keys_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "spec_keys_check '"
        << entry << "', " << *id << "', " << name << "'";

    using namespace tr1::placeholders;

    std::set<UseFlagName> iuse;
    std::transform(id->iuse_key()->value()->begin(),
                   id->iuse_key()->value()->end(),
                   std::inserter(iuse, iuse.begin()),
                   tr1::mem_fn(&IUseFlag::flag));

    CheckForwarder f(entry, reporter, id, iuse, name);
    std::for_each(id->begin_metadata(), id->end_metadata(), tr1::bind(&CheckForwarder::visit_sptr, &f, _1));

    return true;
}

