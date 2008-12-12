/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/unchoices_key.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/validated.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>

using namespace paludis;

namespace
{
    class PALUDIS_VISIBLE UnChoicesKey :
        public MetadataValueKey<std::tr1::shared_ptr<const Choices> >
    {
        private:
            std::tr1::shared_ptr<Choices> _value;

        public:
            UnChoicesKey() :
                MetadataValueKey<std::tr1::shared_ptr<const Choices> >("PALUDIS_CHOICES", "Choices", mkt_internal),
                _value(new Choices)
            {
                _value->add(make_shared_ptr(new Choice("Choices", "Choices", ChoicePrefixName(""), true, true, true, false)));
            }

            ~UnChoicesKey()
            {
            }

            const std::tr1::shared_ptr<const Choices> value() const
            {
                return _value;
            }
    };
}

std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
paludis::unchoices_key()
{
    static std::tr1::shared_ptr<const UnChoicesKey> result(new UnChoicesKey);
    return result;
}

