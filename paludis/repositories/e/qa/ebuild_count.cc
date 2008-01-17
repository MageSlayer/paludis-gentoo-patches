/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include "ebuild_count.hh"
#include <paludis/qa.hh>
#include <paludis/repository.hh>
#include <paludis/util/log.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

using namespace paludis;

bool
paludis::erepository::ebuild_count_check(
        QAReporter & reporter,
        const FSEntry & dir,
        const tr1::shared_ptr<const Repository> & repo,
        const QualifiedPackageName & q,
        const std::string & name
        )
{
    Context context("When performing check '" + name + "' using ebuild_count_check on directory '" + stringify(dir) + "':");
    Log::get_instance()->message(ll_debug, lc_context) << "ebuild_count_check '"
        << dir << "', " << name << "'";

    PackageIDSequence::ConstIterator::difference_type count(
        std::distance(repo->package_ids(q)->begin(), repo->package_ids(q)->end()));

    if (count > 20)
        reporter.message(QAMessage(dir, qaml_minor, name, "Found " + stringify(count) +
                " ebuilds, which is too many to count on both hands and both feet"));
    else if (count > 15)
        reporter.message(QAMessage(dir, qaml_minor, name, "Found " + stringify(count) +
                " ebuilds, which is too many to count on both hands and one foot"));
    else if (count > 10)
        reporter.message(QAMessage(dir, qaml_minor, name, "Found " + stringify(count) +
                " ebuilds, which is too many to count on my fingers"));

    return true;
}

