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

#ifndef PALUDIS_GUARD_PALUDIS_NOTIFIER_CALLBACK_HH
#define PALUDIS_GUARD_PALUDIS_NOTIFIER_CALLBACK_HH 1

#include <paludis/notifier_callback-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/name.hh>
#include <paludis/environment-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE NotifierCallbackEvent :
        public virtual DeclareAbstractAcceptMethods<NotifierCallbackEvent, MakeTypeList<
            NotifierCallbackGeneratingMetadataEvent,
            NotifierCallbackResolverStepEvent,
            NotifierCallbackResolverStageEvent,
            NotifierCallbackLinkageStepEvent>::Type>
    {
    };

    class PALUDIS_VISIBLE NotifierCallbackGeneratingMetadataEvent :
        public NotifierCallbackEvent,
        public ImplementAcceptMethods<NotifierCallbackEvent, NotifierCallbackGeneratingMetadataEvent>
    {
        private:
            const RepositoryName _repo;

        public:
            NotifierCallbackGeneratingMetadataEvent(const RepositoryName & r);

            const RepositoryName repository() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE NotifierCallbackResolverStepEvent :
        public NotifierCallbackEvent,
        public ImplementAcceptMethods<NotifierCallbackEvent, NotifierCallbackResolverStepEvent>
    {
    };

    class PALUDIS_VISIBLE NotifierCallbackResolverStageEvent :
        public NotifierCallbackEvent,
        public ImplementAcceptMethods<NotifierCallbackEvent, NotifierCallbackResolverStageEvent>
    {
        private:
            const std::string _stage;

        public:
            NotifierCallbackResolverStageEvent(const std::string &);

            const std::string stage() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE NotifierCallbackLinkageStepEvent :
        public NotifierCallbackEvent,
        public ImplementAcceptMethods<NotifierCallbackEvent, NotifierCallbackLinkageStepEvent>
    {
        private:
            const FSEntry _location;

        public:
            NotifierCallbackLinkageStepEvent(const FSEntry &);

            const FSEntry location() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    class PALUDIS_VISIBLE ScopedNotifierCallback :
        private PrivateImplementationPattern<ScopedNotifierCallback>
    {
        public:
            ScopedNotifierCallback(Environment * const, const NotifierCallbackFunction &);
            ~ScopedNotifierCallback();

            void remove_now();
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<ScopedNotifierCallback>;
#endif

}

#endif
