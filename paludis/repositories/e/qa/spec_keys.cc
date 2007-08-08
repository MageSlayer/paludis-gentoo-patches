/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/save.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/parallel_for_each.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct Checker :
        ConstVisitor<GenericSpecTree>
    {
        const FSEntry entry;
        QAReporter & reporter;
        const tr1::shared_ptr<const PackageID> & id;
        const MetadataKey & key;
        const std::string name;
        unsigned level;
        bool child_of_any;

        Checker(
                const FSEntry & f,
                QAReporter & r,
                const tr1::shared_ptr<const PackageID> & i,
                const MetadataKey & k,
                const std::string & n) :
            entry(f),
            reporter(r),
            id(i),
            key(k),
            name(n),
            level(0),
            child_of_any(false)
        {
        }

        void visit_leaf(const PackageDepSpec &)
        {
        }

        void visit_leaf(const BlockDepSpec & b)
        {
            if (child_of_any)
                reporter.message(entry, qaml_normal, name, "'|| ( )' block with block child '!"
                        + stringify(*b.blocked_spec()) + "' in spec key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
        }

        void visit_leaf(const URIDepSpec &)
        {
        }

        void visit_leaf(const PlainTextDepSpec &)
        {
        }

        void visit_leaf(const LabelsDepSpec<URILabelVisitorTypes> &)
        {
        }

        void visit_sequence(const UseDepSpec &,
                GenericSpecTree::ConstSequenceIterator cur,
                GenericSpecTree::ConstSequenceIterator end)
        {
            if (child_of_any)
                reporter.message(entry, qaml_normal, name, "'|| ( )' block with use? ( ) child in spec key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");

            Save<unsigned> save_level(&level, level + 1);
            Save<bool> save_child_of_any(&child_of_any, false);
            if (cur == end)
                reporter.message(entry, qaml_normal, name, "Empty 'use? ( )' block in spec key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
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
                    reporter.message(entry, qaml_normal, name, "Empty '( )' block in spec key '" + stringify(key.raw_name())
                            + "' for ID '" + stringify(*id) + "'");
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
                reporter.message(entry, qaml_normal, name, "Empty '|| ( )' block in spec key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
            else if (next(cur) == end)
            {
                cur->accept(*this);
                reporter.message(entry, qaml_normal, name, "'|| ( )' block with only one child in spec key '" + stringify(key.raw_name())
                        + "' for ID '" + stringify(*id) + "'");
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
        const tr1::shared_ptr<const PackageID> & id;
        const std::string name;

        CheckForwarder(
                const FSEntry & f,
                QAReporter & r,
                const tr1::shared_ptr<const PackageID> & i,
                const std::string & n) :
            entry(f),
            reporter(r),
            id(i),
            name(n)
        {
        }

        void visit(const MetadataStringKey &)
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

        void visit(const MetadataSetKey<IUseFlagSet> &)
        {
        }

        void visit(const MetadataSetKey<KeywordNameSet> &)
        {
        }

        void visit(const MetadataSetKey<InheritedSet> &)
        {
        }

        void visit(const MetadataSetKey<UseFlagNameSet> &)
        {
        }

        void visit(const MetadataSetKey<PackageIDSequence> &)
        {
        }

        void visit(const MetadataSpecTreeKey<URISpecTree> & k)
        {
            Context context("When visiting metadata key '" + k.raw_name() + "':");
            Checker c(entry, reporter, id, k, name);
            k.value()->accept(c);
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            Context context("When visiting metadata key '" + k.raw_name() + "':");
            Checker c(entry, reporter, id, k, name);
            k.value()->accept(c);
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            Context context("When visiting metadata key '" + k.raw_name() + "':");
            Checker c(entry, reporter, id, k, name);
            k.value()->accept(c);
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
            Context context("When visiting metadata key '" + k.raw_name() + "':");
            Checker c(entry, reporter, id, k, name);
            k.value()->accept(c);
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
        {
            Context context("When visiting metadata key '" + k.raw_name() + "':");
            Checker c(entry, reporter, id, k, name);
            k.value()->accept(c);
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

    CheckForwarder f(entry, reporter, id, name);
    parallel_for_each(indirect_iterator(id->begin_metadata()), indirect_iterator(id->end_metadata()), accept_visitor(f));

    return true;
}

