/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/validated.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_named_values.hh>

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
                _value(new Choices)
            {
                _value->add(make_shared_ptr(new Choice(make_named_values<ChoiceParams>(
                                    value_for<n::consider_added_or_changed>(false),
                                    value_for<n::contains_every_value>(true),
                                    value_for<n::hidden>(true),
                                    value_for<n::human_name>("Choices"),
                                    value_for<n::prefix>(ChoicePrefixName("")),
                                    value_for<n::raw_name>("Choices"),
                                    value_for<n::show_with_no_prefix>(true)
                                    ))));
            }

            ~UnChoicesKey()
            {
            }

            const std::tr1::shared_ptr<const Choices> value() const
            {
                return _value;
            }

            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "Choices";
            }

            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return "PALUDIS_CHOICES";
            }

            virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result))
            {
                return mkt_internal;
            }
    };
}

std::tr1::shared_ptr<const MetadataValueKey<std::tr1::shared_ptr<const Choices> > >
paludis::unchoices_key()
{
    static std::tr1::shared_ptr<const UnChoicesKey> result(new UnChoicesKey);
    return result;
}

