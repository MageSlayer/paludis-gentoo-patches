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

#ifndef PALUDIS_GUARD_PALUDIS_COUNTED_PTR_HH
#define PALUDIS_GUARD_PALUDIS_COUNTED_PTR_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/comparison_policy.hh>
#include <paludis/util/exception.hh>

/** \file
 * Declaration for the CountedPtr template class.
 *
 * \ingroup grppointers
 */

namespace paludis
{
    /**
     * Contains CountedPtr count policies.
     *
     * \ingroup grppointers
     */
    namespace count_policy
    {
        /**
         * CountedPtr policy: reference counts are stored separately.
         *
         * \ingroup grppointers
         */
        struct ExternalCountTag
        {
        };

        /**
         * CountedPtr policy: reference counts are stored by the class via the
         * Counted subclass.
         *
         * \ingroup grppointers
         */
        struct InternalCountTag
        {
        };
    }

    /**
     * CountedPtr internals.
     *
     * \ingroup grppointers
     */
    namespace counted_ptr_internals
    {
        /**
         * Base class for CountedPtr.
         *
         * \ingroup grppointers
         * \nosubgrouping
         */
        template <typename T_>
        class CountedPtrBase :
            public ComparisonPolicy<CountedPtrBase<T_>,
                       comparison_mode::EqualityComparisonTag,
                       comparison_method::CompareByMemberTag<T_ *> >
        {
            private:
                CountedPtrBase(const CountedPtrBase & other);

                const CountedPtrBase & operator= (const CountedPtrBase & other);

            protected:
                /**
                 * Pointer to our data.
                 */
                T_ * _ptr;

                ///\name Basic operations
                ///\{

                CountedPtrBase(T_ * ptr) :
                    ComparisonPolicy<CountedPtrBase<T_>,
                        comparison_mode::EqualityComparisonTag,
                        comparison_method::CompareByMemberTag<T_ *> >(
                                &CountedPtrBase::_ptr),
                    _ptr(ptr)
                {
                }

                virtual ~CountedPtrBase()
                {
                }

                ///\}

            public:
                ///\name Dereference operators
                ///\{

                inline const T_ & operator* () const;
                inline const T_ * operator-> () const;
                T_ & operator* ();
                T_ * operator-> ();

                /**
                 * Fetch our raw pointer.
                 */
                T_ * raw_pointer() const
                {
                    return _ptr;
                }

                ///\}

                /**
                 * Return whether we are null. We use const void * rather than bool
                 * here to avoid bool -> int conversion weirdness. See \ref
                 * TCppPL 21.3.3.
                 */
                operator const void * () const
                {
                    return _ptr;
                }
        };

        template <typename T_>
        const T_ & CountedPtrBase<T_>::operator* () const
        {
            return *_ptr;
        }

        template <typename T_>
        const T_ * CountedPtrBase<T_>::operator-> () const
        {
            return _ptr;
        }

        template <typename T_>
        T_ & CountedPtrBase<T_>::operator* ()
        {
            return *_ptr;
        }

        template <typename T_>
        T_ * CountedPtrBase<T_>::operator-> ()
        {
            return _ptr;
        }
    }

    /**
     * Reference counted pointer class.
     *
     * \ingroup grppointers
     */
    template <typename T_, typename CountPolicy_ = count_policy::InternalCountTag>
    class CountedPtr;

    /**
     * Base for an internal counted class.
     *
     * \ingroup grppointers
     */
    template <typename T_>
    class InternalCounted;

    /**
     * Reference counted pointer class (specialisation for ExternalCountTag).
     *
     * \ingroup grppointers
     * \nosubgrouping
     */
    template <typename T_>
    class CountedPtr<T_, count_policy::ExternalCountTag> :
        public counted_ptr_internals::CountedPtrBase<T_>
    {
        private:
            unsigned * _ref_count;

        public:
            ///\name Basic operations
            ///\{

            explicit CountedPtr(T_ * const ptr) :
                counted_ptr_internals::CountedPtrBase<T_>(ptr),
                _ref_count(new unsigned(1))
            {
            }

            CountedPtr(const CountedPtr & other) :
                counted_ptr_internals::CountedPtrBase<T_>(other.raw_pointer()),
                _ref_count(other._ref_count)
            {
                ++*_ref_count;
            }

            /**
             * Constructor, from another CountedPtr of a descendent of our
             * data's class.
             */
            template <typename O_>
            CountedPtr(const CountedPtr<O_, count_policy::ExternalCountTag> & other) :
                counted_ptr_internals::CountedPtrBase<T_>(other._ptr),
                _ref_count(other.reference_count_pointer())
            {
                ++*_ref_count;
            }

            ~CountedPtr()
            {
                if (0 == --(*_ref_count))
                {
                    delete this->_ptr;
                    delete _ref_count;
                }
            }

            /**
             * Assignment, from another CountedPtr.
             */
            const CountedPtr & operator= (const CountedPtr & other)
            {
                if (other._ptr != this->_ptr)
                {
                    if (0 == --*_ref_count)
                    {
                        delete this->_ptr;
                        delete _ref_count;
                    }

                    this->_ptr = other._ptr;
                    _ref_count = other._ref_count;
                    ++*_ref_count;
                }
                return *this;
            }

            /**
             * Explicit assignment, from a raw pointer.
             */
            const CountedPtr & assign(T_ * const other)
            {
                return operator= (CountedPtr<T_, count_policy::ExternalCountTag>(other));
            }

            /**
             * Explicit assignment to zero.
             */
            const CountedPtr & zero()
            {
                return operator= (CountedPtr<T_, count_policy::ExternalCountTag>(0));
            }

            ///\}

            ///\name Pointer operations
            ///\{

            /**
             * Fetch our reference count pointer.
             */
            unsigned * reference_count_pointer() const
            {
                return _ref_count;
            }

            ///\}
    };

    /**
     * Reference counted pointer class (specialisation for InternalCountTag).
     *
     * \ingroup grppointers
     * \nosubgrouping
     */
    template <typename T_>
    class CountedPtr<T_, count_policy::InternalCountTag> :
        public counted_ptr_internals::CountedPtrBase<T_>
    {
        public:
            ///\name Basic operations
            ///\{

            explicit CountedPtr(T_ * const ptr) :
                counted_ptr_internals::CountedPtrBase<T_>(ptr)
            {
                if (0 != this->_ptr)
                    ++*this->_ptr->reference_count_pointer();
            }

            CountedPtr(const CountedPtr & other) :
                counted_ptr_internals::CountedPtrBase<T_>(other._ptr)
            {
                if (0 != this->_ptr)
                    ++*this->_ptr->reference_count_pointer();
            }

            /**
             * Constructor, from another CountedPtr of a descendent of our
             * data's class.
             */
            template <typename O_>
            CountedPtr(const CountedPtr<O_, count_policy::InternalCountTag> & other) :
                counted_ptr_internals::CountedPtrBase<T_>(
                        static_cast<T_ *>(other.raw_pointer()))
            {
                if (0 != this->_ptr)
                    ++*this->_ptr->reference_count_pointer();
            }

            ~CountedPtr();

            const CountedPtr & operator= (const CountedPtr & other)
            {
                if (other._ptr != this->_ptr)
                {
                    if (0 != this->_ptr)
                        if (0 == --(*this->_ptr->reference_count_pointer()))
                            delete this->_ptr;

                    this->_ptr = other._ptr;
                    if (0 != this->_ptr)
                        ++*this->_ptr->reference_count_pointer();
                }
                return *this;
            }

            /**
             * Explicit assignment, from a raw pointer.
             */
            const CountedPtr & assign(T_ * const other)
            {
                return operator= (CountedPtr<T_, count_policy::InternalCountTag>(other));
            }

            /**
             * Explicit assignment to zero.
             */
            const CountedPtr & zero()
            {
                return operator= (CountedPtr<T_, count_policy::InternalCountTag>(0));
            }

            ///\}

            ///\name Pointer operations
            ///\{

            /**
             * Fetch our reference count pointer.
             */
            unsigned * reference_count_pointer() const
            {
                if (0 != this->_ptr)
                    return this->_ptr->reference_count_pointer();
                else
                    return 0;
            }

            ///\}
    };

    template <typename T_>
    CountedPtr<T_, count_policy::InternalCountTag>::~CountedPtr()
    {
        if (0 != this->_ptr)
            if (0 == --(*this->_ptr->reference_count_pointer()))
                delete this->_ptr;
    }
}

#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
    /**
     * Base class for an internally counted class.
     *
     * \ingroup grppointers
     * \nosubgrouping
     */
    template <typename T_>
    class InternalCounted :
        private InstantiationPolicy<InternalCounted<T_>,
            instantiation_method::NonCopyableTag>
    {
        private:
            mutable unsigned _ref_count;

        protected:
            ///\name Basic operations
            ///\{

            InternalCounted() :
                _ref_count(0)
            {
            }

            ~InternalCounted()
            {
            }

            ///\}

        public:
            ///\name Pointer types
            ///\{

            /**
             * A CountedPtr to us.
             */
            typedef CountedPtr<T_, count_policy::InternalCountTag> Pointer;

            /**
             * A CountedPtr to us (const).
             */
            typedef CountedPtr<const T_, count_policy::InternalCountTag> ConstPointer;

            ///\}

            ///\name Pointer operations
            ///\{

            /**
             * Fetch a pointer to our reference count (may be zero).
             */
            unsigned * reference_count_pointer() const
            {
                return & _ref_count;
            }

            ///\}
    };
}

#endif

