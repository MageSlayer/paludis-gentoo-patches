/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/e_choice_value.hh>
#include <paludis/repositories/e/use_desc.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/pool-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    class EChoiceValue :
        public ChoiceValue
    {
        private:
            const EChoiceValueParams _params;

        public:
            EChoiceValue(const EChoiceValueParams &);

            const UnprefixedChoiceName unprefixed_name() const;
            const ChoiceNameWithPrefix name_with_prefix() const;
            bool enabled() const;
            bool enabled_by_default() const;
            bool locked() const;
            const std::string description() const;
            ChoiceOrigin origin() const;
            const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result));
            const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const PALUDIS_ATTRIBUTE((warn_unused_result));
            bool presumed() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

EChoiceValue::EChoiceValue(const EChoiceValueParams & p) :
    _params(p)
{
}

const UnprefixedChoiceName
EChoiceValue::unprefixed_name() const
{
    return _params.unprefixed_choice_name();
}

const ChoiceNameWithPrefix
EChoiceValue::name_with_prefix() const
{
    return _params.choice_name_with_prefix();
}

const std::string
EChoiceValue::description() const
{
    return _params.description();
}

bool
EChoiceValue::enabled() const
{
    return _params.enabled();
}

bool
EChoiceValue::enabled_by_default() const
{
    return _params.enabled_by_default();
}

bool
EChoiceValue::locked() const
{
    return _params.locked();
}

ChoiceOrigin
EChoiceValue::origin() const
{
    return _params.origin();
}

const std::string
EChoiceValue::parameter() const
{
    return "";
}

const std::shared_ptr<const PermittedChoiceValueParameterValues>
EChoiceValue::permitted_parameter_values() const
{
    return nullptr;
}

bool
EChoiceValue::presumed() const
{
    return _params.presumed();
}

namespace paludis
{
    template <>
    struct Hash<EChoiceValueParams>
    {
        Hash<ChoiceNameWithPrefix> choice_name_with_prefix_hash;
        Hash<ChoicePrefixName> choice_prefix_name_hash;
        Hash<std::string> description_hash;
        Hash<bool> enabled_hash;
        Hash<bool> enabled_by_default_hash;
        Hash<bool> origin_hash;
        Hash<bool> locked_hash;
        Hash<UnprefixedChoiceName> unprefixed_choice_name_hash;

        std::size_t operator() (const EChoiceValueParams & p) const
        {
            return 0
                ^ choice_name_with_prefix_hash(p.choice_name_with_prefix())
                ^ choice_prefix_name_hash(p.choice_prefix_name())
                ^ description_hash(p.description())
                ^ enabled_hash(p.enabled())
                ^ enabled_by_default_hash(p.enabled_by_default())
                ^ origin_hash(p.origin())
                ^ locked_hash(p.locked())
                ^ unprefixed_choice_name_hash(p.unprefixed_choice_name())
                ;
        }
    };

    bool operator== (const EChoiceValueParams & a, const EChoiceValueParams & b)
    {
        return true
            && (a.choice_name_with_prefix() == b.choice_name_with_prefix())
            && (a.choice_prefix_name() == b.choice_prefix_name())
            && (a.description() == b.description())
            && (a.enabled() == b.enabled())
            && (a.enabled_by_default() == b.enabled_by_default())
            && (a.origin() == b.origin())
            && (a.locked() == b.locked())
            && (a.presumed() == b.presumed())
            && (a.unprefixed_choice_name() == b.unprefixed_choice_name())
            ;
    }
}

const std::shared_ptr<const ChoiceValue>
paludis::erepository::create_e_choice_value(const EChoiceValueParams & p)
{
    return Pool<EChoiceValue>::get_instance()->create(p);
}

