/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda
 * Copyright (c) 2008 David Leverton
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

#include <iostream>
#include "manifest.hh"
#include <paludis/qa.hh>
#include <paludis/spec_tree.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/member_iterator-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/sha1.hh>
#include <paludis/util/sha256.hh>
#include <paludis/util/rmd160.hh>
#include <paludis/util/md5.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/layout.hh>
#include <paludis/repositories/e/manifest2_reader.hh>
#include <fstream>
#include <set>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    struct DistfilesCollector
    {
        std::tr1::shared_ptr<const PackageID> id;
        std::map<std::string, std::tr1::shared_ptr<PackageIDSet> > & distfiles;

        DistfilesCollector(const std::tr1::shared_ptr<const PackageID> & i,
                           std::map<std::string, std::tr1::shared_ptr<PackageIDSet> > & d) :
            id(i),
            distfiles(d)
        {
        }

        void visit(const FetchableURISpecTree::NodeType<AllDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const FetchableURISpecTree::NodeType<ConditionalDepSpec>::Type & node)
        {
            std::for_each(indirect_iterator(node.begin()), indirect_iterator(node.end()), accept_visitor(*this));
        }

        void visit(const FetchableURISpecTree::NodeType<URILabelsDepSpec>::Type &)
        {
        }

        void visit(const FetchableURISpecTree::NodeType<FetchableURIDepSpec>::Type & node)
        {
            std::map<std::string, std::tr1::shared_ptr<PackageIDSet> >::iterator it(distfiles.find(node.spec()->filename()));
            if (distfiles.end() == it)
            {
                std::tr1::shared_ptr<PackageIDSet> set(new PackageIDSet);
                it = distfiles.insert(std::make_pair(node.spec()->filename(), set)).first;
            }
            it->second->insert(id);
        }
    };

    struct Manifest2Checker
    {
        QAReporter & reporter;
        FSEntry dir;
        std::string name;

        FSEntry manifest;
        std::tr1::shared_ptr<Map<FSEntry, std::string> > files;
        std::tr1::shared_ptr<const PackageIDSequence> packages;

        std::set<FSEntry> accounted_files;
        std::map<std::string, std::tr1::shared_ptr<PackageIDSet> > distfiles;
        std::set<std::string> accounted_distfiles;

        Manifest2Checker(QAReporter & r, const FSEntry & d, const std::string & n,
                         const FSEntry & m, const std::tr1::shared_ptr<Map<FSEntry, std::string> > & f,
                         const std::tr1::shared_ptr<const PackageIDSequence> & p) :
            reporter(r),
            dir(d),
            name(n),
            manifest(m),
            files(f),
            packages(p)
        {
        }

        void check_file(const Manifest2Entry & entry)
        {
            if ("DIST" == entry.type())
            {
                if (distfiles.end() == distfiles.find(entry.name()))
                {
                    QAMessage m(manifest, qaml_minor, name, "DIST file '" + entry.name() + "' is not used by any package");
                    for (PackageIDSequence::ConstIterator it(packages->begin()),
                             it_end(packages->end()); it_end != it; ++it)
                    {
                        m = m.with_associated_id(*it);
                        if ((*it)->fetches_key())
                            m = m.with_associated_key(*it, (*it)->fetches_key());
                    }
                    reporter.message(m);
                }

                accounted_distfiles.insert(entry.name());
                return;
            }

            FSEntry file("AUX" == entry.type() ? dir / "files" / entry.name() : dir / entry.name());
            Map<FSEntry, std::string>::ConstIterator it(files->find(file));
            if (files->end() == it)
            {
                reporter.message(QAMessage(file, qaml_normal, name, "File is listed in the Manifest but is either not present or should not be listed"));
                return;
            }

            if (entry.type() != it->second)
                reporter.message(QAMessage(file, qaml_normal, name, "File is of type '" + it->second + "', but Manifest lists '" + entry.type() + "'"));
            if (entry.size() != file.file_size())
                reporter.message(QAMessage(file, qaml_normal, name, "File size is '" + stringify(file.file_size()) + "', but Manifest lists '" + stringify(entry.size()) + "'"));

            if (! entry.sha1().empty())
            {
                std::ifstream s(stringify(file).c_str());
                SHA1 sha1(s);
                if (entry.sha1() != sha1.hexsum())
                    reporter.message(QAMessage(file, qaml_normal, name, "File SHA1 is '" + sha1.hexsum() + "', but Manifest lists '" + entry.sha1() + "'"));

            }

            if (! entry.sha256().empty())
            {
                std::ifstream s(stringify(file).c_str());
                SHA256 sha256(s);
                if (entry.sha256() != sha256.hexsum())
                    reporter.message(QAMessage(file, qaml_normal, name, "File SHA256 is '" + sha256.hexsum() + "', but Manifest lists '" + entry.sha256() + "'"));

            }

            if (! entry.rmd160().empty())
            {
                std::ifstream s(stringify(file).c_str());
                RMD160 rmd160(s);
                if (entry.rmd160() != rmd160.hexsum())
                    reporter.message(QAMessage(file, qaml_normal, name, "File RMD160 is '" + rmd160.hexsum() + "', but Manifest lists '" + entry.rmd160() + "'"));

            }

            if (! entry.md5().empty())
            {
                std::ifstream s(stringify(file).c_str());
                MD5 md5(s);
                if (entry.md5() != md5.hexsum())
                    reporter.message(QAMessage(file, qaml_normal, name, "File MD5 is '" + md5.hexsum() + "', but Manifest lists '" + entry.md5() + "'"));

            }

            accounted_files.insert(it->first);
        }

        void check_unmanifested()
        {
            std::set<FSEntry> stray_files;
            std::set_difference(first_iterator(files->begin()), first_iterator(files->end()),
                                accounted_files.begin(), accounted_files.end(),
                                std::inserter(stray_files, stray_files.end()));

            for (std::set<FSEntry>::const_iterator file_it(stray_files.begin()),
                     file_it_end(stray_files.end()); file_it_end != file_it; ++file_it)
                reporter.message(QAMessage(*file_it, qaml_normal, name, "File is not listed in the Manifest"));

            std::set<std::string> stray_distfiles;
            std::set_difference(first_iterator(distfiles.begin()), first_iterator(distfiles.end()),
                                accounted_distfiles.begin(), accounted_distfiles.end(),
                                std::inserter(stray_distfiles, stray_distfiles.end()));

            for (std::set<std::string>::const_iterator dist_it(stray_distfiles.begin()),
                     dist_it_end(stray_distfiles.end()); dist_it_end != dist_it; ++dist_it)
            {
                std::tr1::shared_ptr<const PackageIDSet> set(distfiles.find(*dist_it)->second);
                for (PackageIDSet::ConstIterator pkg_it(set->begin()),
                         pkg_it_end(set->end()); pkg_it_end != pkg_it; ++pkg_it)
                    reporter.message(QAMessage((*pkg_it)->fs_location_key()->value(), qaml_normal, name, "DIST file '" + *dist_it + "' is not listed in the Manifest")
                            .with_associated_id(*pkg_it)
                            .with_associated_key(*pkg_it, (*pkg_it)->fetches_key()));
            }
        }
    };
}

bool
paludis::erepository::manifest_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const std::tr1::shared_ptr<const ERepository> & repo,
        const QualifiedPackageName & qpn,
        const std::string & name
        )
{
    using namespace std::tr1::placeholders;

    Context context("When performing check '" + name + "' using manifest_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message("e.qa.manifest_check", ll_debug, lc_context) << "manifest_check '"
        << dir << "', " << name << "'";

    try
    {
        FSEntry manifest(dir / "Manifest");

        if (! manifest.is_regular_file())
        {
            reporter.message(QAMessage(manifest, qaml_normal, name, "Manifest is missing or not a regular file"));
            return true;
        }

        std::tr1::shared_ptr<const PackageIDSequence> ids(repo->package_ids(qpn));
        Manifest2Checker checker(reporter, dir, name, manifest, repo->layout()->manifest_files(qpn), ids);
        for (PackageIDSequence::ConstIterator it(ids->begin()),
                 it_end(ids->end()); it_end != it; ++it)
        {
            if ((*it)->fetches_key())
            {
                DistfilesCollector c(*it, checker.distfiles);
                (*it)->fetches_key()->value()->root()->accept(c);
            }
        }

        Manifest2Reader reader(manifest);
        std::for_each(reader.begin(), reader.end(), std::tr1::bind(&Manifest2Checker::check_file, std::tr1::ref(checker), _1));
        checker.check_unmanifested();

        bool is_signed(false);
        {
            std::ifstream ff(stringify(manifest).c_str());
            if (! ff)
            {
                reporter.message(QAMessage(manifest, qaml_normal, name, "Can't read Manifest file"));
                return true;
            }

            std::string s;
            if (std::getline(ff, s))
                is_signed = (0 == s.compare("-----BEGIN PGP SIGNED MESSAGE-----"));
        }

        if (is_signed)
        {
            int status(run_command("gpg --verify " + stringify(manifest) + " >/dev/null 2>/dev/null"));

            if (1 == status)
                reporter.message(QAMessage(manifest, qaml_normal, name, "Broken Manifest signature"));
            else if (2 == status)
                reporter.message(QAMessage(manifest, qaml_maybe, name, "Manifest signature cannot be verified"));
        }
        else
            reporter.message(QAMessage(manifest, qaml_minor, name, "Manifest not signed"));
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & e)
    {
        reporter.message(QAMessage(dir, qaml_severe, name,
                    "Caught exception '" + stringify(e.message()) + "' ("
                    + stringify(e.what()) + ") when checking Manifest for '" + stringify(qpn) + "'"));
    }

    return true;
}

