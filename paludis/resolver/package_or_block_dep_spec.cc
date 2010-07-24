/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/sequence-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/serialise-impl.hh>
#include <paludis/metadata_key.hh>
#include <paludis/elike_package_dep_spec.hh>
#include <paludis/elike_annotations.hh>
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
    if_package(n::if_package() = make_null_shared_ptr())
{
}

PackageOrBlockDepSpec::PackageOrBlockDepSpec(const PackageDepSpec & s) :
    if_block(n::if_block() = make_null_shared_ptr()),
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
            .member(SerialiserFlags<>(), "strong", if_block()->strong())
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

    if (! spec->annotations_key())
        w.member(SerialiserFlags<>(), "annotations_count", 0);
    else
    {
        int n(0);
        for (MetadataSectionKey::MetadataConstIterator m(spec->annotations_key()->begin_metadata()),
                m_end(spec->annotations_key()->end_metadata()) ;
                m != m_end ; ++m)
        {
            const MetadataValueKey<std::string> * k(
                    simple_visitor_cast<const MetadataValueKey<std::string> >(**m));
            if (! k)
            {
                Log::get_instance()->message("resolver.sanitised_dependencies.not_a_string", ll_warning, lc_context)
                    << "Annotation '" << (*m)->raw_name() << "' not a string. This is probably a bug.";
                continue;
            }

            w.member(SerialiserFlags<>(), "annotations_k_" + stringify(n), k->human_name());
            w.member(SerialiserFlags<>(), "annotations_v_" + stringify(n), k->value());
            ++n;
        }

        w.member(SerialiserFlags<>(), "annotations_count", stringify(n));
    }
}

PackageOrBlockDepSpec
PackageOrBlockDepSpec::deserialise(Deserialisation & d, const std::shared_ptr<const PackageID> & for_id)
{
    Context context("When deserialising:");

    Deserialisator v(d, "PackageOrBlockDepSpec");

    bool block(v.member<bool>("block"));
    PackageDepSpec spec(parse_elike_package_dep_spec(v.member<std::string>("spec"),
                { epdso_allow_tilde_greater_deps, epdso_nice_equal_star,
                epdso_allow_ranged_deps, epdso_allow_use_deps, epdso_allow_use_deps_portage,
                epdso_allow_use_dep_defaults, epdso_allow_repository_deps, epdso_allow_slot_star_deps,
                epdso_allow_slot_equal_deps, epdso_allow_slot_deps, epdso_allow_key_requirements,
                epdso_allow_use_dep_question_defaults },
                { vso_flexible_dashes, vso_flexible_dots, vso_ignore_case,
                vso_letters_anywhere, vso_dotted_suffixes },
                for_id));

    std::shared_ptr<MetadataSectionKey> annotations;

    std::shared_ptr<Map<std::string, std::string> > m(std::make_shared<Map<std::string, std::string>>());
    for (int a(0), a_end(v.member<int>("annotations_count")) ;
            a != a_end ; ++a)
    {
        std::string key(v.member<std::string>("annotations_k_" + stringify(a)));
        std::string value(v.member<std::string>("annotations_v_" + stringify(a)));
        m->insert(key, value);
    }

    if (! m->empty())
        annotations = std::make_shared<ELikeAnnotations>(m);

    if (block)
    {
        bool strong(v.member<bool>("strong"));
        std::string text(v.member<std::string>("text"));
        BlockDepSpec b_spec(text, spec, strong);
        if (annotations)
            b_spec.set_annotations_key(annotations);
        return PackageOrBlockDepSpec(b_spec);
    }
    else
    {
        if (annotations)
            spec.set_annotations_key(annotations);
        return PackageOrBlockDepSpec(spec);
    }
}

template class Sequence<PackageOrBlockDepSpec>;
template class WrappedForwardIterator<Sequence<PackageOrBlockDepSpec>::ConstIteratorTag, const PackageOrBlockDepSpec>;

