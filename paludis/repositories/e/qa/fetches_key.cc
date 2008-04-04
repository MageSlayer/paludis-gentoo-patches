/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "fetches_key.hh"
#include <paludis/qa.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/log.hh>
#include <paludis/util/save.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_label.hh>
#include <paludis/package_id.hh>
#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;

namespace
{
    struct LabelToFetchRestrict : ConstVisitor<URILabelVisitorTypes>
    {
        bool value;

        LabelToFetchRestrict(const URILabel & l)
        {
            l.accept(*this);
        }

        void visit(const URIMirrorsThenListedLabel &)
        {
            value = false;
        }

        void visit(const URIMirrorsOnlyLabel &)
        {
            value = true;
        }

        void visit(const URIListedOnlyLabel &)
        {
            value = false;
        }

        void visit(const URIListedThenMirrorsLabel &)
        {
            value = false;
        }

        void visit(const URILocalMirrorsOnlyLabel &)
        {
            value = true;
        }

        void visit(const URIManualOnlyLabel &)
        {
            value = true;
        }
    };

    struct Checker : ConstVisitor<FetchableURISpecTree>
    {
        QAReporter & reporter;
        bool fetch_restrict;
        tr1::shared_ptr<const PackageID> id;
        tr1::shared_ptr<const MetadataKey> key;
        FSEntry entry;
        std::string name;

        Checker(QAReporter & rr, bool f,
                const tr1::shared_ptr<const PackageID> & i,
                const tr1::shared_ptr<const MetadataKey> & k,
                const FSEntry & fs, const std::string & n) :
            reporter(rr),
            fetch_restrict(f),
            id(i),
            key(k),
            entry(fs),
            name(n)
        {
        }

        void visit_leaf(const FetchableURIDepSpec & a)
        {
            std::string::size_type p(std::string::npos);
            if (std::string::npos == ((p = a.original_url().find("://"))) &&  ! fetch_restrict)
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "No protocol found for '" + a.original_url() +
                            "' and not fetch restricted in '" + key->raw_name() + "'")
                        .with_associated_id(id)
                        .with_associated_key(id, key));

            else if ((std::string::npos != p) &&
                    (("http" != a.original_url().substr(0, p)) &&
                     ("https" != a.original_url().substr(0, p)) &&
                     ("mirror" != a.original_url().substr(0, p)) &&
                     ("ftp" != a.original_url().substr(0, p))))
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Unrecognised protocol for '" + a.original_url() +
                            "' in '" + key->raw_name() + "'")
                        .with_associated_id(id)
                        .with_associated_key(id, key));

            else if ((std::string::npos != a.original_url().find("dev.gentoo.org")) ||
                     (std::string::npos != a.original_url().find("cvs.gentoo.org")) ||
                     (std::string::npos != a.original_url().find("toucan.gentoo.org")) ||
                     (std::string::npos != a.original_url().find("emu.gentoo.org")) ||
                     (std::string::npos != a.original_url().find("alpha.gnu.org")) ||
                     (std::string::npos != a.original_url().find("geocities.com")))
                reporter.message(QAMessage(entry, qaml_normal, name,
                            "Unreliable host for '" + a.original_url() + "' in '" +
                            key->raw_name() + "'")
                        .with_associated_id(id)
                        .with_associated_key(id, key));

            else
            {
                if (0 == a.original_url().compare(0, 9, "mirror://"))
                {
                    std::string mirror_host(a.original_url().substr(9));
                    std::string::size_type pos(mirror_host.find('/'));
                    if (std::string::npos == pos)
                        reporter.message(QAMessage(entry, qaml_normal, name,
                                    "Malformed component '" + a.original_url() + "' in '" +
                                    key->raw_name() + "'")
                                .with_associated_id(id)
                                .with_associated_key(id, key));
                    else
                    {
                        mirror_host.erase(pos);
                        RepositoryMirrorsInterface * m((*id->repository())[k::mirrors_interface()]);
                        if (! m->is_mirror(mirror_host))
                            reporter.message(QAMessage(entry, qaml_normal, name,
                                        "Unknown mirror '" + mirror_host + "' for '" +
                                        a.original_url() + "' in '" + key->raw_name() + "'")
                                    .with_associated_id(id)
                                    .with_associated_key(id, key));
                    }
                }
            }
        }

        void visit_leaf(const URILabelsDepSpec & a)
        {
            if (1 != std::distance(a.begin(), a.end()))
                throw InternalError(PALUDIS_HERE, "URILabelsDepSpec contains " +
                        stringify(std::distance(a.begin(), a.end())) + " labels, but expected 1");
            fetch_restrict = LabelToFetchRestrict(**a.begin()).value;
        }

        void visit_sequence(const AllDepSpec &,
                FetchableURISpecTree::ConstSequenceIterator begin,
                FetchableURISpecTree::ConstSequenceIterator end)
        {
            Save<bool> s(&fetch_restrict);
            std::for_each(begin, end, accept_visitor(*this));
        }

        void visit_sequence(const ConditionalDepSpec &,
                FetchableURISpecTree::ConstSequenceIterator begin,
                FetchableURISpecTree::ConstSequenceIterator end)
        {
            Save<bool> s(&fetch_restrict);
            std::for_each(begin, end, accept_visitor(*this));
        }
    };
}

bool
paludis::erepository::fetches_key_check(
        const FSEntry & entry,
        QAReporter & reporter,
        const tr1::shared_ptr<const PackageID> & id,
        const std::string & name)
{
    Context context("When performing check '" + name + "' using fetches_key_check on ID '" + stringify(*id) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "fetches_key_check '"
        << entry << "', " << *id << "', " << name << "'";

    if (id->fetches_key())
    {
        try
        {
            Checker c(reporter, LabelToFetchRestrict(*id->fetches_key()->initial_label()).value,
                      id, id->fetches_key(), entry, name);
            id->fetches_key()->value()->accept(c);
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

