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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_INTEREST_IN_SPEC_HELPER_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_INTEREST_IN_SPEC_HELPER_HH 1

#include <paludis/resolver/interest_in_spec_helper-fwd.hh>
#include <paludis/resolver/resolution-fwd.hh>
#include <paludis/resolver/sanitised_dependencies-fwd.hh>
#include <paludis/resolver/resolver_functions-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/tribool-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE InterestInSpecHelper
        {
            private:
                Pimp<InterestInSpecHelper> _imp;

            public:
                explicit InterestInSpecHelper(const Environment * const);
                ~InterestInSpecHelper();

                void add_take_spec(const PackageDepSpec &);
                void add_take_group(const std::string &);
                void add_take_from_spec(const PackageDepSpec &);
                void add_ignore_spec(const PackageDepSpec &);
                void add_ignore_group(const std::string &);
                void add_ignore_from_spec(const PackageDepSpec &);
                void add_no_blockers_from_spec(const PackageDepSpec &);
                void add_no_dependencies_from_spec(const PackageDepSpec &);

                void set_follow_installed_dependencies(const bool);
                void set_follow_installed_build_dependencies(const bool);

                void set_take_suggestions(const Tribool);
                void set_take_recommendations(const Tribool);
                void set_cross_compile_host(const std::string &);

                SpecInterest operator() (
                        const std::shared_ptr<const Resolution> &,
                        const std::shared_ptr<const PackageID> &,
                        const SanitisedDependency &) const;
        };
    }

    extern template class Pimp<resolver::InterestInSpecHelper>;
}

#endif
