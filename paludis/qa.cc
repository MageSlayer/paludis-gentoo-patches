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

#include <paludis/qa.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/set-impl.hh>

using namespace paludis;

#include <paludis/qa-se.cc>
#include <paludis/qa-sr.cc>

template class Set<tr1::shared_ptr<const PackageID>, PackageIDSetComparator>;
template class Sequence<tr1::shared_ptr<const MetadataKey> >;

QAReporter::~QAReporter()
{
}

tr1::shared_ptr<PackageIDSet>
QAMessage::default_associated_ids()
{
    return tr1::shared_ptr<PackageIDSet>(new PackageIDSet);
}

tr1::shared_ptr<QAMessage::KeysSequence>
QAMessage::default_associated_keys()
{
    return tr1::shared_ptr<KeysSequence>(new KeysSequence);
}

QAMessage &
QAMessage::with_associated_id(const tr1::shared_ptr<const PackageID> & id)
{
    associated_ids->insert(id);
    return *this;
}

QAMessage &
QAMessage::with_associated_key(const tr1::shared_ptr<const PackageID> & id,
                               const tr1::shared_ptr<const MetadataKey> & k)
{
    associated_keys->push_back(std::make_pair(id, k));
    return *this;
}

