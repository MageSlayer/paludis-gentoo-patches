/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_FACTORY_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_FACTORY_HH 1

#include <paludis/environment_factory-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/no_type.hh>
#include <paludis/util/singleton.hh>
#include <paludis/environment-fwd.hh>
#include <memory>
#include <functional>

namespace paludis
{
    class PALUDIS_VISIBLE FallBackToAnotherFormatError
    {
    };

    /**
     * Factory for Environment creation.
     *
     * \ingroup g_environment
     * \since 0.30
     */
    class PALUDIS_VISIBLE EnvironmentFactory :
        private PrivateImplementationPattern<EnvironmentFactory>,
        public Singleton<EnvironmentFactory>
    {
        friend class Singleton<EnvironmentFactory>;

        private:
            EnvironmentFactory();
            ~EnvironmentFactory();

        public:
            typedef std::function<const std::shared_ptr<Environment>(const std::string &)> CreateFunction;

            /**
             * Create an Environment subclass from the specified spec.
             *
             * \param spec The environment spec, which is in the form
             *     env:suffix, where env is the string representing an
             *     Environment's kind (e.g. "paludis", "portage") and
             *     suffix is the information to pass to the constructing
             *     function (for paludis, a config suffix, and for portage,
             *     a location). If env is not specified, it defaults to
             *     trying paludis then portage. If suffix is not specified,
             *     it defaults to an empty string. If no colon is present,
             *     the supplied string is taken as env (this includes an
             *     empty string).
             */
            const std::shared_ptr<Environment> create(const std::string & spec) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Add a repository format.
             *
             * \param formats must have at least one value, and no value may be
             * specified more than once across all invocations.
             *
             * \param create_function is used to implement EnvironmentFactory::create.
             */
            void add_environment_format(
                    const std::shared_ptr<const Set<std::string> > & formats,
                    const CreateFunction & create_function
                    );
    };

    template <typename EnvironmentClass_>
    void register_environment(const EnvironmentClass_ * const, EnvironmentFactory * const);

    extern template class PrivateImplementationPattern<EnvironmentFactory>;
    extern template class Singleton<EnvironmentFactory>;
}

#endif
