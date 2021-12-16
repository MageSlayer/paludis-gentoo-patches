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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_SUGGEST_RESTART_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_SUGGEST_RESTART_HH 1

#include <paludis/resolver/suggest_restart-fwd.hh>
#include <paludis/resolver/resolvent-fwd.hh>
#include <paludis/resolver/constraint-fwd.hh>
#include <paludis/resolver/decision-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE SuggestRestart :
            public Exception
        {
            private:
                Pimp<SuggestRestart> _imp;

            public:
                SuggestRestart(
                        const Resolvent &,
                        const std::shared_ptr<const Decision> & previous_decision,
                        const std::shared_ptr<const Constraint> & problematic_constraint,
                        const std::shared_ptr<const Decision> & new_decision,
                        const std::shared_ptr<const Constraint> & suggested_preset) noexcept;
                SuggestRestart(const SuggestRestart &);
                ~SuggestRestart() override;

                const Resolvent resolvent() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<const Decision> previous_decision() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<const Constraint> problematic_constraint() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<const Decision> new_decision() const PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::shared_ptr<const Constraint> suggested_preset() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<resolver::SuggestRestart>;
}

#endif
