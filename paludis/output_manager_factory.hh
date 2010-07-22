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

#ifndef PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_FACTORY_HH
#define PALUDIS_GUARD_PALUDIS_OUTPUT_MANAGER_FACTORY_HH 1

#include <paludis/output_manager_factory-fwd.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <functional>
#include <memory>

namespace paludis
{
    class PALUDIS_VISIBLE OutputManagerFactory :
        private PrivateImplementationPattern<OutputManagerFactory>,
        public Singleton<OutputManagerFactory>
    {
        friend class Singleton<OutputManagerFactory>;

        private:
            OutputManagerFactory();
            ~OutputManagerFactory();

        public:
            typedef std::function<std::string (const std::string &)> KeyFunction;

            typedef std::function<const std::shared_ptr<OutputManager> (
                    const std::string &)> CreateChildFunction;

            typedef std::function<std::string (
                    const std::string & pattern,
                    const std::shared_ptr<const Map<std::string, std::string> > & extra_vars)> ReplaceVarsFunc;

            typedef std::function<const std::shared_ptr<OutputManager>(
                    const KeyFunction &,
                    const CreateChildFunction &,
                    const ReplaceVarsFunc &
                    )> CreateFunction;

            /**
             * Construct a given OutputManager, or throw ConfigurationError.
             *
             * \param key_function should return the value for a given key. The
             * 'handler' key must return a value (e.g. 'standard'), which is used
             * to select the return type. Other key names are manager defined,
             * but typically include things like 'location' and 'keep_on_success'.
             *
             * \param create_child_function is used by, for example,
             * TeeOutputManager to create child streams. Given a single string,
             * this function returns the appropriate child.
             *
             * \param replace_vars_func should replace '%{vars}' with their
             * expanded forms. Variables include 'name', 'action' etc. This is
             * used by, for example, FileOutputManager to allow the user to
             * specify the output file names.
             */
            const std::shared_ptr<OutputManager> create(
                    const KeyFunction & key_function,
                    const CreateChildFunction & create_child_function,
                    const ReplaceVarsFunc & replace_vars_func
                    ) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Add a manager.
             *
             * \param managers must have at least one value, and no value may be
             * specified more than once across all invocations.
             *
             * \param create_function is used to implement OutputManagerFactory::create.
             */
            void add_manager(
                    const std::shared_ptr<const Set<std::string> > & managers,
                    const CreateFunction & create_function
                    );

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::string> ConstIterator;
            ConstIterator begin_keys() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end_keys() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PrivateImplementationPattern<OutputManagerFactory>;
    extern template class Singleton<OutputManagerFactory>;
    extern template class WrappedForwardIterator<OutputManagerFactory::ConstIteratorTag, const std::string>;
}

#endif
