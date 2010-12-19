/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/legacy/override_functions.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/environment.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/mask.hh>
#include <paludis/name.hh>

using namespace paludis;

bool
paludis::override_tilde_keywords(const Environment * const e, const PackageID & id, const Mask & m)
{
    Context c("When working out whether mask is a tilde keyword mask for override:");

    const UnacceptedMask * const mm(simple_visitor_cast<const UnacceptedMask>(m));
    if (! mm)
        return false;

    const MetadataCollectionKey<KeywordNameSet> * const k(simple_visitor_cast<const MetadataCollectionKey<KeywordNameSet> >(*mm->unaccepted_key()));
    if (! k)
        return false;

    std::shared_ptr<KeywordNameSet> kk(std::make_shared<KeywordNameSet>());
    for (KeywordNameSet::ConstIterator i(k->value()->begin()), i_end(k->value()->end()) ;
            i != i_end ; ++i)
    {
        kk->insert(*i);
        if ('~' == stringify(*i).at(0))
            kk->insert(KeywordName(stringify(*i).substr(1)));
    }

    return e->accept_keywords(kk, id);
}

bool
paludis::override_unkeyworded(const Environment * const e, const PackageID & id, const Mask & m)
{
    Context c("When working out whether mask is an unkeyworded mask for override:");

    const UnacceptedMask * const mm(simple_visitor_cast<const UnacceptedMask>(m));
    if (! mm)
        return false;

    const MetadataCollectionKey<KeywordNameSet> * const k(simple_visitor_cast<const MetadataCollectionKey<KeywordNameSet> >(*mm->unaccepted_key()));
    if (! k)
        return false;

    std::shared_ptr<KeywordNameSet> kk(std::make_shared<KeywordNameSet>());
    for (KeywordNameSet::ConstIterator i(k->value()->begin()), i_end(k->value()->end()) ;
            i != i_end ; ++i)
        if ('-' == stringify(*i).at(0))
            kk->insert(KeywordName(stringify(*i).substr(1)));

    return ! e->accept_keywords(kk, id);
}

bool
paludis::override_repository_masks(const Mask & m)
{
    Context c("When working out whether mask is a repository mask for override:");
    return simple_visitor_cast<const RepositoryMask>(m);
}

bool
paludis::override_license(const Mask & m)
{
    Context c("When working out whether mask is a license mask for override:");
    const UnacceptedMask * const mm(simple_visitor_cast<const UnacceptedMask>(m));
    return mm && simple_visitor_cast<const MetadataSpecTreeKey<LicenseSpecTree> >(*mm->unaccepted_key());
}


