/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/spec_rewriter.hh>
#include <paludis/resolver/sanitised_dependencies.hh>
#include <paludis/resolver/resolvent.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/spec_tree.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/elike_blocker.hh>
#include <map>
#include <set>

#include "config.h"

using namespace paludis;
using namespace paludis::resolver;

typedef std::map<QualifiedPackageName, std::set<QualifiedPackageName> > Rewrites;

const std::shared_ptr<const DependencySpecTree>
RewrittenSpec::as_spec_tree() const
{
    const std::shared_ptr<DependencySpecTree> result(std::make_shared<DependencySpecTree>(std::make_shared<AllDepSpec>()));

    for (Sequence<PackageOrBlockDepSpec>::ConstIterator i(specs()->begin()), i_end(specs()->end()) ;
            i != i_end ; ++i)
        if (i->if_package())
            result->top()->append(i->if_package());
        else
            result->top()->append(i->if_block());

    return result;
}

namespace paludis
{
    template <>
    struct Imp<SpecRewriter>
    {
        const Environment * const env;

        mutable Mutex rewrites_mutex;
        mutable Rewrites rewrites;
        mutable bool has_rewrites;

        Imp(const Environment * const e) :
            env(e),
            has_rewrites(false)
        {
        }
    };
}

SpecRewriter::SpecRewriter(const Environment * const e) :
    _imp(e)
{
}

SpecRewriter::~SpecRewriter() = default;

const std::shared_ptr<const RewrittenSpec>
SpecRewriter::rewrite_if_special(const PackageOrBlockDepSpec & s, const std::shared_ptr<const Resolvent> & maybe_our_resolvent) const
{
    if (s.if_package() && s.if_package()->package_ptr())
    {
        if (s.if_package()->package_ptr()->category() != CategoryNamePart("virtual"))
            return make_null_shared_ptr();
        _need_rewrites();

        Rewrites::const_iterator r(_imp->rewrites.find(*s.if_package()->package_ptr()));
        if (r == _imp->rewrites.end())
            return make_null_shared_ptr();

        const std::shared_ptr<RewrittenSpec> result(std::make_shared<RewrittenSpec>(make_named_values<RewrittenSpec>(
                        n::specs() = std::make_shared<Sequence<PackageOrBlockDepSpec>>()
                        )));

        for (std::set<QualifiedPackageName>::const_iterator n(r->second.begin()), n_end(r->second.end()) ;
                n != n_end ; ++n)
            result->specs()->push_back(PackageOrBlockDepSpec(PartiallyMadePackageDepSpec(*s.if_package()).package(*n)));

        return result;
    }
    else if (s.if_block() && s.if_block()->blocking().package_ptr())
    {
        if (s.if_block()->blocking().package_ptr()->category() != CategoryNamePart("virtual"))
            return make_null_shared_ptr();
        _need_rewrites();

        Rewrites::const_iterator r(_imp->rewrites.find(*s.if_block()->blocking().package_ptr()));
        if (r == _imp->rewrites.end())
            return make_null_shared_ptr();

        const std::shared_ptr<RewrittenSpec> result(std::make_shared<RewrittenSpec>(make_named_values<RewrittenSpec>(
                        n::specs() = std::make_shared<Sequence<PackageOrBlockDepSpec>>()
                        )));

        for (std::set<QualifiedPackageName>::const_iterator n(r->second.begin()), n_end(r->second.end()) ;
                n != n_end ; ++n)
        {
            if (maybe_our_resolvent && (*n == maybe_our_resolvent->package()))
                continue;

            PackageDepSpec spec(PartiallyMadePackageDepSpec(s.if_block()->blocking()).package(*n));
            auto p(split_elike_blocker(s.if_block()->text()));
            BlockDepSpec b(std::get<1>(p) + stringify(spec), spec);
            b.set_annotations(s.if_block()->maybe_annotations());
            result->specs()->push_back(b);
        }

        return result;
    }
    else
        return make_null_shared_ptr();
}

void
SpecRewriter::_need_rewrites() const
{
#ifdef ENABLE_VIRTUALS_REPOSITORY
    Lock lock(_imp->rewrites_mutex);
    if (_imp->has_rewrites)
        return;
    _imp->has_rewrites = true;

    Context context("When performing virtuals rewrites:");

    const std::shared_ptr<const PackageIDSequence> ids((*_imp->env)[selection::AllVersionsSorted(
                generator::InRepository(RepositoryName("virtuals")) +
                generator::InRepository(RepositoryName("installed-virtuals"))
                )]);
    for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
            i != i_end ; ++i)
    {
        if (! ((*i)->virtual_for_key()))
            throw InternalError(PALUDIS_HERE, "huh? " + stringify(**i) + " has no virtual_for_key");
        _imp->rewrites.insert(std::make_pair((*i)->name(), std::set<QualifiedPackageName>())).first->second.insert(
                (*i)->virtual_for_key()->parse_value()->name());
    }
#endif
}

template class Pimp<resolver::SpecRewriter>;

