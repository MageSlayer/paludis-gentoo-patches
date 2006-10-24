/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/attributes.hh>

/** \file
 * InstantiationPolicy patterns.
 *
 * \ingroup grpinstance
 */

namespace paludis
{
    /**
     * Instantiation policies for paludis::InstantiationPolicy.
     *
     * \ingroup grpinstance
     */
    namespace instantiation_method
    {
        /**
         * Cannot be copied or assigned to.
         *
         * \ingroup grpinstance
         */
        struct NonCopyableTag
        {
        };

        /**
         * Cannot be instantiated
         *
         * \ingroup grpinstance
         */
        struct NonInstantiableTag
        {
        };

        /**
         * Single instance created at startup.
         *
         * \ingroup grpinstance
         */
        struct SingletonAtStartupTag
        {
        };

        /**
         * Single instance created when needed.
         *
         * \ingroup grpinstance
         */
        struct SingletonAsNeededTag
        {
        };
    }

    /**
     * InstantiationPolicy is used to specify behaviour of classes that have
     * something other than the default C++ instantiation behaviour.
     *
     * \ingroup grpinstance
     */
    template <typename OurType_, typename InstantiationMethodTag_>
    struct InstantiationPolicy;

    /**
     * InstantiationPolicy: specialisation for classes that cannot be copied
     * or assigned to.
     *
     * \ingroup grpinstance
     * \nosubgrouping
     */
    template<typename OurType_>
    class PALUDIS_VISIBLE InstantiationPolicy<OurType_, instantiation_method::NonCopyableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

        protected:
            ///\name Basic operations
            ///\{

            ~InstantiationPolicy()
            {
            }

            InstantiationPolicy()
            {
            }

            ///\}
    };

    /**
     * InstantiationPolicy: specialisation for classes that cannot be created.
     *
     * \ingroup grpinstance
     * \nosubgrouping
     */
    template<typename OurType_>
    class InstantiationPolicy<OurType_, instantiation_method::NonInstantiableTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

        protected:
            ///\name Basic operations
            ///\{

            InstantiationPolicy();

            ~InstantiationPolicy()
            {
            }

            ///\}
    };
}

#include <paludis/util/counted_ptr.hh>

namespace paludis
{
    /**
     * InstantiationPolicy: specialisation for singleton classes that are
     * created at startup.
     *
     * \ingroup grpinstance
     * \nosubgrouping
     */
    template<typename OurType_>
    class PALUDIS_VISIBLE InstantiationPolicy<OurType_, instantiation_method::SingletonAtStartupTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

            static CountedPtr<OurType_, count_policy::ExternalCountTag> _instance;

        protected:
            ///\name Basic operations
            ///\{

            InstantiationPolicy()
            {
            }

            ///\}

        public:
            ///\name Singleton operations
            ///\{

            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance()
            {
                return _instance.raw_pointer();
            }

            ///\}
    };

    template <typename OurType_>
    CountedPtr<OurType_, count_policy::ExternalCountTag>
    InstantiationPolicy<OurType_, instantiation_method::SingletonAtStartupTag>::_instance(
            new OurType_);

    /**
     * InstantiationPolicy: specialisation for singleton classes that are
     * created as needed.
     *
     * \ingroup grpinstance
     * \nosubgrouping
     */
    template<typename OurType_>
    class PALUDIS_VISIBLE InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>
    {
        private:
            InstantiationPolicy(const InstantiationPolicy &);

            const InstantiationPolicy & operator= (const InstantiationPolicy &);

            static OurType_ * * _get_instance_ptr();

            class DeleteOnDestruction;
            friend class DeleteOnDestruction;

            static void _delete(OurType_ * const p)
            {
                delete p;
            }

            class PALUDIS_VISIBLE DeleteOnDestruction
            {
                private:
                    OurType_ * * const _ptr;

                public:
                    DeleteOnDestruction(OurType_ * * const p) :
                        _ptr(p)
                    {
                    }

                    ~DeleteOnDestruction()
                    {
                        InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>::_delete(* _ptr);
                        * _ptr = 0;
                    }
            };

        protected:
            ///\name Basic operations
            ///\{

            InstantiationPolicy()
            {
            }

            ///\}

        public:
            ///\name Singleton operations
            ///\{

            /**
             * Fetch our instance.
             */
            static OurType_ * get_instance();

            /**
             * Destroy our instance.
             */
            static void destroy_instance();

            ///\}
    };

    template<typename OurType_>
    OurType_ * *
    InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>::_get_instance_ptr()
    {
        static OurType_ * instance(0);
        static DeleteOnDestruction delete_instance(&instance);

        return &instance;
    }

    template<typename OurType_>
    OurType_ *
    InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>::get_instance()
    {
        OurType_ * * i(_get_instance_ptr());
        if (0 == *i)
            *i = new OurType_;
        return *i;
    }

    template<typename OurType_>
    void
    InstantiationPolicy<OurType_, instantiation_method::SingletonAsNeededTag>::destroy_instance()
    {
        OurType_ * * i(_get_instance_ptr());
        delete *i;
        *i = 0;
    }
}

#endif
