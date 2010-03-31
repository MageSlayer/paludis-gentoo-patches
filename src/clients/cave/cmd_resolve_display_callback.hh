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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_DISPLAY_CALLBACK_HH
#define PALUDIS_GUARD_SRC_CLIENTS_CAVE_CMD_RESOLVE_DISPLAY_CALLBACK_HH 1

#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/notifier_callback-fwd.hh>

namespace paludis
{
    namespace cave
    {
        struct ResolverRestart
        {
        };

        class DisplayCallback :
            private PrivateImplementationPattern<DisplayCallback>
        {
            private:
                void update() const;

            public:
                DisplayCallback(const std::string & initial_stage);
                ~DisplayCallback();

                void operator() (const NotifierCallbackEvent & event) const;

                void operator() (const ResolverRestart &) const;

                void visit(const NotifierCallbackGeneratingMetadataEvent &) const;

                void visit(const NotifierCallbackResolverStepEvent &) const;

                void visit(const NotifierCallbackResolverStageEvent &) const;

                void visit(const NotifierCallbackLinkageStepEvent &) const;
        };
    }
}

#endif
