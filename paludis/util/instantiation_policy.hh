/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_INSTANTIATION_POLICY_HH
#define PALUDIS_GUARD_PALUDIS_INSTANTIATION_POLICY_HH 1

namespace paludis
{
    /**
     * Instantiation policies for paludis::InstantiationPolicy.
     */
    namespace instantiation_method
    {
        /**
         * Cannot be copied or assigned to.
         */
        struct NonCopyableTag
        {
        };

        /**
         * Cannot be instantiated.
         */
        struct NonInstantiableTag
        {
        };

        /**
         * Single instance created at startup.
         */
        struct SingletonAtStartupTag
        {
        };

        /**
         * Single instance created when needed.
         */
        struct SingletonAsNeededTag
        {
        };
    }

#ifdef DOXYGEN
    /**
     * InstantiationPolicy is used to specify behaviour of classes that have
     * something other than the default C++ instantiation behaviour.
     */
    template <typename OurType_, typename InstantiationMethodTag_>
    struct InstantiationPolicy
    {
    };
#else
    template <typename OurType_, typename InstantiationMethodTag_>
    struct InstantiationPolicy;
#endif

    /**
     * InstantiationPolicy: specialisation for classes that cannot be copied
     * or assigned to.
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::NonCopyableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

        protected:
            ~InstantiationPolicy()
            {
            }

            InstantiationPolicy()
            {
            }
    };

    /**
     * InstantiationPolicy: specialisation for classes that cannot be created.
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::NonInstantiableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

        protected:
            InstantiationPolicy();

            ~InstantiationPolicy()
            {
            }
    };
}

#include <paludis/util/counted_ptr.hh>

namespace paludis
{
    /**
     * InstantiationPolicy: specialisation for singleton classes that are
     * created at startup.
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::SingletonAtStartupTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

            static CountedPtr<OurType_, count_policy::ExternalCountTag> _instance;

        protected:
            InstantiationPolicy()
            {
            }

        public:
            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance()
            {
                return _instance.raw_pointer();
            }
    };

    template <typename OurType_>
    CountedPtr<OurType_, count_policy::ExternalCountTag>
    InstantiationPolicy<OurType_, instantiation_method::SingletonAtStartupTag>::_instance(
            new OurType_);

    /**
     * InstantiationPolicy: specialisation for singleton classes that are
     * created as needed.
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

            static OurType_ * _instance;

        protected:
            InstantiationPolicy()
            {
            }

        public:
            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance();
    };

    template<typename OurType_>
    OurType_ *
    InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>::get_instance()
    {
        static OurType_ instance;
        return &instance;
    }
}

#endif
