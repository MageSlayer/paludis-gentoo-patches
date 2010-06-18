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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_REQUIRED_CONFIRMATIONS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_REQUIRED_CONFIRMATIONS_HH 1

#include <paludis/resolver/required_confirmations-fwd.hh>
#include <paludis/util/simple_visitor.hh>
#include <paludis/util/type_list.hh>
#include <paludis/util/attributes.hh>
#include <paludis/serialise-fwd.hh>

namespace paludis
{
    namespace resolver
    {
        class PALUDIS_VISIBLE RequiredConfirmation :
            public virtual DeclareAbstractAcceptMethods<RequiredConfirmation, MakeTypeList<
                DowngradeConfirmation, NotBestConfirmation, BreakConfirmation, RemoveSystemPackageConfirmation>::Type>
        {
            public:
                virtual void serialise(Serialiser &) const = 0;

                static const std::tr1::shared_ptr<RequiredConfirmation> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE DowngradeConfirmation :
            public RequiredConfirmation,
            public ImplementAcceptMethods<RequiredConfirmation, DowngradeConfirmation>
        {
            public:
                static const std::tr1::shared_ptr<DowngradeConfirmation> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE NotBestConfirmation :
            public RequiredConfirmation,
            public ImplementAcceptMethods<RequiredConfirmation, NotBestConfirmation>
        {
            public:
                static const std::tr1::shared_ptr<NotBestConfirmation> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE BreakConfirmation :
            public RequiredConfirmation,
            public ImplementAcceptMethods<RequiredConfirmation, BreakConfirmation>
        {
            public:
                static const std::tr1::shared_ptr<BreakConfirmation> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };

        class PALUDIS_VISIBLE RemoveSystemPackageConfirmation :
            public RequiredConfirmation,
            public ImplementAcceptMethods<RequiredConfirmation, RemoveSystemPackageConfirmation>
        {
            public:
                static const std::tr1::shared_ptr<RemoveSystemPackageConfirmation> deserialise(
                        Deserialisation & d) PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual void serialise(Serialiser &) const;
        };
    }
}

#endif
