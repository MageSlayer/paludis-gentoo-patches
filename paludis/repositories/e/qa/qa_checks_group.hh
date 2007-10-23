/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_QA_CHECKS_GROUP_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_QA_QA_CHECKS_GROUP_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>
#include <string>

namespace paludis
{
    namespace erepository
    {
        template <typename T_>
        class QAChecksGroup :
            private PrivateImplementationPattern<QAChecksGroup<T_> >,
            public InstantiationPolicy<QAChecksGroup<T_>, instantiation_method::NonCopyableTag>
        {
            private:
                using PrivateImplementationPattern<QAChecksGroup<T_> >::_imp;
                void need_ordering() const;

            public:
                QAChecksGroup();
                ~QAChecksGroup();

                typedef libwrapiter::ForwardIterator<QAChecksGroup<T_>, T_> ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                void add_check(const std::string &, const T_ &);
                void add_prerequirement(const std::string &, const std::string &);
        };
    }
}

#endif
