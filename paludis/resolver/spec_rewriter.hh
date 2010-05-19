/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_SPEC_REWRITER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_SPEC_REWRITER_HH 1

#include <paludis/resolver/spec_rewriter-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/spec_tree-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct specs_name> specs;
    }

    namespace resolver
    {
        struct RewrittenSpec
        {
            NamedValue<n::specs, std::tr1::shared_ptr<Sequence<PackageOrBlockDepSpec> > > specs;

            const std::tr1::shared_ptr<const DependencySpecTree> as_spec_tree() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE SpecRewriter :
            private PrivateImplementationPattern<SpecRewriter>
        {
            private:
                void _need_rewrites() const;

            public:
                SpecRewriter(const Environment * const);
                ~SpecRewriter();

                const std::tr1::shared_ptr<const RewrittenSpec> rewrite_if_special(const PackageOrBlockDepSpec &,
                        const std::tr1::shared_ptr<const Resolvent> & maybe_from) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<resolver::SpecRewriter>;
#endif

}

#endif
