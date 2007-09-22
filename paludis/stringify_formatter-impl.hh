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

#ifndef PALUDIS_GUARD_PALUDIS_STRINGIFY_FORMATTER_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_STRINGIFY_FORMATTER_IMPL_HH 1

#include <paludis/stringify_formatter.hh>
#include <paludis/util/tr1_type_traits.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>

namespace paludis
{
    template <>
    struct Implementation<StringifyFormatter>
    {
        const CanFormat<IUseFlag> * const f_iuse;
        const CanFormat<UseFlagName> * const f_use;
        const CanFormat<KeywordName> * const f_keyword;
        const CanFormat<PackageDepSpec> * const f_package;
        const CanFormat<BlockDepSpec> * const f_block;
        const CanFormat<URIDepSpec> * const f_uri;
        const CanFormat<LabelsDepSpec<DependencyLabelVisitorTypes> > * const f_dep_label;
        const CanFormat<LabelsDepSpec<URILabelVisitorTypes> > * const f_uri_label;
        const CanFormat<PlainTextDepSpec> * const f_plain;
        const CanFormat<UseDepSpec> * const f_use_dep;
        const CanSpace * const f_space;

        Implementation(
                const CanFormat<IUseFlag> * const f_iuse_v,
                const CanFormat<UseFlagName> * const f_use_v,
                const CanFormat<KeywordName> * const f_keyword_v,
                const CanFormat<PackageDepSpec> * const f_package_v,
                const CanFormat<BlockDepSpec> * const f_block_v,
                const CanFormat<URIDepSpec> * const f_uri_v,
                const CanFormat<LabelsDepSpec<DependencyLabelVisitorTypes> > * const f_dep_label_v,
                const CanFormat<LabelsDepSpec<URILabelVisitorTypes> > * const f_uri_label_v,
                const CanFormat<PlainTextDepSpec> * const f_plain_v,
                const CanFormat<UseDepSpec> * const f_use_dep_v,
                const CanSpace * const f_space_v
                ) :
            f_iuse(f_iuse_v),
            f_use(f_use_v),
            f_keyword(f_keyword_v),
            f_package(f_package_v),
            f_block(f_block_v),
            f_uri(f_uri_v),
            f_dep_label(f_dep_label_v),
            f_uri_label(f_uri_label_v),
            f_plain(f_plain_v),
            f_use_dep(f_use_dep_v),
            f_space(f_space_v)
        {
        }
    };

    template <bool b_, typename T_>
    struct StringifyFormatterGetForwarder
    {
        static const CanFormat<T_> * get(const CanFormat<T_> * const t)
        {
            return t;
        }
    };

    template <typename T_>
    struct StringifyFormatterGetForwarder<false, T_>
    {
        static const CanFormat<T_> * get(const void * const)
        {
            return 0;
        }
    };

    template <bool b_>
    struct StringifyFormatterGetSpaceForwarder
    {
        static const CanSpace * get(const CanSpace * const t)
        {
            return t;
        }
    };

    template <>
    struct StringifyFormatterGetSpaceForwarder<false>
    {
        static const CanSpace * get(const void * const)
        {
            return 0;
        }
    };

    template <typename T_>
        StringifyFormatter::StringifyFormatter(const T_ & t) :
            PrivateImplementationPattern<StringifyFormatter>(new Implementation<StringifyFormatter>(
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<IUseFlag> *>::value, IUseFlag>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<UseFlagName> *>::value, UseFlagName>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<KeywordName> *>::value, KeywordName>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<PackageDepSpec> *>::value, PackageDepSpec>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<BlockDepSpec> *>::value, BlockDepSpec>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<URIDepSpec> *>::value, URIDepSpec>::get(&t),
                        StringifyFormatterGetForwarder<
                            tr1::is_convertible<T_ *, CanFormat<LabelsDepSpec<DependencyLabelVisitorTypes> > *>::value,
                            LabelsDepSpec<DependencyLabelVisitorTypes> >::get(&t),
                        StringifyFormatterGetForwarder<
                            tr1::is_convertible<T_ *, CanFormat<LabelsDepSpec<URILabelVisitorTypes> > *>::value,
                            LabelsDepSpec<URILabelVisitorTypes> >::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<PlainTextDepSpec> *>::value, PlainTextDepSpec>::get(&t),
                        StringifyFormatterGetForwarder<tr1::is_convertible<T_ *, CanFormat<UseDepSpec> *>::value, UseDepSpec>::get(&t),
                        StringifyFormatterGetSpaceForwarder<tr1::is_convertible<T_ *, CanSpace *>::value>::get(&t)
                        ))
    {
    }
}

#endif
