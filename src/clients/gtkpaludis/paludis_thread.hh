/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_PALUDIS_THREAD_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_PALUDIS_THREAD_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <glibmm/dispatcher.h>
#include <sigc++/trackable.h>
#include <sigc++/slot.h>
#include <tr1/memory>

namespace gtkpaludis
{
    class PaludisThread :
        public paludis::InstantiationPolicy<PaludisThread, paludis::instantiation_method::SingletonTag>,
        private paludis::PrivateImplementationPattern<PaludisThread>,
        public sigc::trackable
    {
        friend class paludis::InstantiationPolicy<PaludisThread, paludis::instantiation_method::SingletonTag>;

        public:
            class Launchable :
                private paludis::InstantiationPolicy<Launchable, paludis::instantiation_method::NonCopyableTag>
            {
                friend class PaludisThread;

                protected:
                    Launchable();

                public:
                    void dispatch(const sigc::slot<void> &);

                    virtual ~Launchable();
                    virtual void operator() () = 0;

                    class StatusBarMessage
                    {
                        private:
                            Launchable * const _l;

                        public:
                            StatusBarMessage(Launchable * const, const std::string &);
                            ~StatusBarMessage();
                    };
            };

            friend class Launchable;

        private:
            PaludisThread();
            virtual ~PaludisThread();

            void _queue_add(const sigc::slot<void> &);
            void _queue_run();
            void _thread_func(std::tr1::shared_ptr<Launchable>);

        public:
            void launch(std::tr1::shared_ptr<Launchable>);
            bool try_lock_unlock();
    };
}

#endif
