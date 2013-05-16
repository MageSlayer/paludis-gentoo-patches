/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#include <paludis/resolver/package_or_block_dep_spec.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/dep_spec_annotations.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <ostream>

using namespace paludis;
using namespace paludis::resolver;

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const PackageOrBlockDepSpec & d)
{
    if (d.if_package())
        return s << *d.if_package();
    else
        return s << *d.if_block();
}

PackageOrBlockDepSpec::PackageOrBlockDepSpec(const BlockDepSpec & s) :
    if_block(n::if_block() = std::make_shared<BlockDepSpec>(s)),
    if_package(n::if_package() = nullptr)
{
}

PackageOrBlockDepSpec::PackageOrBlockDepSpec(const PackageDepSpec & s) :
    if_block(n::if_block() = nullptr),
    if_package(n::if_package() = std::make_shared<PackageDepSpec>(s))
{
}

void
PackageOrBlockDepSpec::serialise(Serialiser & s) const
{
    SerialiserObjectWriter w(s.object("PackageOrBlockDepSpec"));

    std::shared_ptr<DepSpec> spec;
    if (if_block())
    {
        spec = if_block();
        w
            .member(SerialiserFlags<>(), "block", true)
            .member(SerialiserFlags<>(), "spec", stringify(if_block()->blocking()))
            .member(SerialiserFlags<>(), "text", if_block()->text())
            ;
    }
    else
    {
        spec = if_package();
        w
            .member(SerialiserFlags<>(), "block", false)
            .member(SerialiserFlags<>(), "spec", stringify(*if_package()))
            ;
    }

    if (! spec->maybe_annotations())
        w.member(SerialiserFlags<>(), "annotations_count", 0);
    else
    {
        int n(0);
        for (auto m(spec->maybe_annotations()->begin()), m_end(spec->maybe_annotations()->end()) ;
                m != m_end ; ++m)
        {
            w.member(SerialiserFlags<>(), "annotations_k_" + stringify(n), m->key());
            w.member(SerialiserFlags<>(), "annotations_v_" + stringify(n), m->value());
            w.member(SerialiserFlags<>(), "annotations_n_" + stringify(n), stringify(m->kind()));
            w.member(SerialiserFlags<>(), "annotations_r_" + stringify(n), stringify(m->role()));
            ++n;
        }

        w.member(SerialiserFlags<>(), "annotations_count", stringify(n));
    }
}

PackageOrBlockDepSpec
PackageOrBlockDepSpec::deserialise(Deserialisation & d, const std::shared_ptr<const PackageID> &)
{
    Context context("When deserialising:");

    Deserialisator v(d, "PackageOrBlockDepSpec");

    bool block(v.member<bool>("block"));
    PackageDepSpec spec(parse_elike_package_dep_spec(v.member<std::string>("spec"),
                { epdso_allow_tilde_greater_deps, epdso_nice_equal_star,
                epdso_allow_ranged_deps, epdso_allow_use_deps, epdso_allow_use_deps_portage,
                epdso_allow_use_dep_defaults, epdso_allow_repository_deps, epdso_allow_slot_star_deps,
                epdso_allow_slot_equal_deps, epdso_allow_slot_equal_deps_portage,
                epdso_allow_slot_deps, epdso_allow_key_requirements,
                epdso_allow_use_dep_question_defaults, epdso_allow_subslot_deps },
                { vso_flexible_dashes, vso_flexible_dots, vso_ignore_case,
                vso_letters_anywhere, vso_dotted_suffixes }));

    auto annotations(std::make_shared<DepSpecAnnotations>());
    for (int a(0), a_end(v.member<int>("annotations_count")) ;
            a != a_end ; ++a)
    {
        std::string key(v.member<std::string>("annotations_k_" + stringify(a)));
        std::string value(v.member<std::string>("annotations_v_" + stringify(a)));
        std::string kind(v.member<std::string>("annotations_n_" + stringify(a)));
        std::string role(v.member<std::string>("annotations_r_" + stringify(a)));
        annotations->add(make_named_values<DepSpecAnnotation>(
                    n::key() = key,
                    n::kind() = destringify<DepSpecAnnotationKind>(kind),
                    n::role() = destringify<DepSpecAnnotationRole>(role),
                    n::value() = value
                    ));
    }

    if (block)
    {
        std::string text(v.member<std::string>("text"));
        BlockDepSpec b_spec(text, spec);
        if (annotations->begin() != annotations->end())
            b_spec.set_annotations(annotations);
        return PackageOrBlockDepSpec(b_spec);
    }
    else
    {
        if (annotations->begin() != annotations->end())
            spec.set_annotations(annotations);
        return PackageOrBlockDepSpec(spec);
    }
}

namespace paludis
{
    template class Sequence<PackageOrBlockDepSpec>;
    template class WrappedForwardIterator<Sequence<PackageOrBlockDepSpec>::ConstIteratorTag, const PackageOrBlockDepSpec>;
}
