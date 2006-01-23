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

#ifndef PALUDIS_GUARD_PALUDIS_COUNTED_PTR_HH
#define PALUDIS_GUARD_PALUDIS_COUNTED_PTR_HH 1

#include <paludis/attributes.hh>
#include <paludis/comparison_policy.hh>
#include <paludis/exception.hh>

/** \file
 * Declaration for the CountedPtr template class.
 *
 * \ingroup Pointer
 * \ingroup Exception
 */

namespace paludis
{
    /**
     * Thrown when a CountedPtr check fails.
     *
     * \ingroup Pointer
     * \ingroup Exception
     */
    class CountedPtrError : public Exception
    {
        public:
            /**
             * Constructor.
             */
            CountedPtrError() throw ();
    };

    /**
     * Contains CountedPtr count policies.
     *
     * \ingroup Pointer
     */
    namespace count_policy
    {
        /**
         * CountedPtr policy: reference counts are stored separately.
         *
         * \ingroup Pointer
         */
        struct ExternalCountTag
        {
        };

        /**
         * CountedPtr policy: reference counts are stored by the class via the
         * Counted subclass.
         *
         * \ingroup Pointer
         */
        struct InternalCountTag
        {
        };
    }

    /**
     * Contains CountedPtr dereference policies.
     *
     * \ingroup Pointer
     */
    namespace dereference_policy
    {
        /**
         * CountedPtr dereferences are not checked.
         *
         * \ingroup Pointer
         */
        struct UncheckedDereferenceTag
        {
        };

        /**
         * CountedPtr dereferences are checked, and a CountedPtrError is
         * thrown for 0 dereferences.
         *
         * \ingroup Pointer
         */
        struct CheckedDereferenceTag
        {
        };
    }

    /**
     * CountedPtr internals.
     *
     * \ingroup Pointer
     */
    namespace counted_ptr_internals
    {
        /**
         * Base class for CountedPtr.
         */
        template <typename T_, typename DereferencePolicy_>
        class CountedPtrBase;

        /**
         * Base class for CountedPtr (specialisation for UncheckedDereferenceTag).
         */
        template <typename T_>
        class CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag> :
            public ComparisonPolicy<CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>,
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

                /**
                 * Constructor.
                 */
                CountedPtrBase(T_ * ptr) :
                    ComparisonPolicy<CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>,
                        comparison_mode::EqualityComparisonTag,
                        comparison_method::CompareByMemberTag<T_ *> >(
                                &CountedPtrBase::_ptr),
                    _ptr(ptr)
                {
                }

                /**
                 * Destructor.
                 */
                ~CountedPtrBase()
                {
                }

            public:
                /**
                 * Dereference operator (const).
                 */
                inline const T_ & operator* () const PALUDIS_ATTRIBUTE((pure));

                /**
                 * Dereference to member operator (const).
                 */
                inline const T_ * operator-> () const PALUDIS_ATTRIBUTE((pure));

                /**
                 * Dereference operator (non const).
                 */
                T_ & operator* () PALUDIS_ATTRIBUTE((pure));

                /**
                 * Dereference to member operator (non const).
                 */
                T_ * operator-> () PALUDIS_ATTRIBUTE((pure));

                /**
                 * Not null?
                 */
                operator bool() const
                {
                    return _ptr;
                }

                /**
                 * Fetch our raw pointer.
                 */
                T_ * raw_pointer() const
                {
                    return _ptr;
                }
        };

        template <typename T_>
        const T_ & CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>::operator* () const
        {
            return *_ptr;
        }

        template <typename T_>
        const T_ * CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>::operator-> () const
        {
            return _ptr;
        }

        template <typename T_>
        T_ & CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>::operator* ()
        {
            return *_ptr;
        }

        template <typename T_>
        T_ * CountedPtrBase<T_, dereference_policy::UncheckedDereferenceTag>::operator-> ()
        {
            return _ptr;
        }
        /**
         * Base class for CountedPtr (specialisation for CheckedDereferenceTag).
         */
        template <typename T_>
        class CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag> :
            public ComparisonPolicy<CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>,
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

                /**
                 * Constructor.
                 */
                CountedPtrBase(T_ * ptr) :
                    ComparisonPolicy<CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>,
                        comparison_mode::EqualityComparisonTag,
                        comparison_method::CompareByMemberTag<T_ *> >(
                                &CountedPtrBase::_ptr),
                    _ptr(ptr)
                {
                }

                /**
                 * Destructor.
                 */
                ~CountedPtrBase()
                {
                }

            public:
                /**
                 * Dereference operator (const).
                 */
                inline const T_ & operator* () const;

                /**
                 * Dereference to member operator (const).
                 */
                inline const T_ * operator-> () const;

                /**
                 * Dereference operator (non const).
                 */
                T_ & operator* ();

                /**
                 * Dereference to member operator (non const).
                 */
                T_ * operator-> ();

                /**
                 * Not null?
                 */
                operator bool() const
                {
                    return _ptr;
                }

                /**
                 * Fetch our raw pointer.
                 */
                T_ * raw_pointer() const
                {
                    return _ptr;
                }
        };

        template <typename T_>
        const T_ & CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>::operator* () const
        {
            if (0 == _ptr)
                throw CountedPtrError();
            return *_ptr;
        }

        template <typename T_>
        const T_ * CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>::operator-> () const
        {
            if (0 == _ptr)
                throw CountedPtrError();
            return _ptr;
        }

        template <typename T_>
        T_ & CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>::operator* ()
        {
            if (0 == _ptr)
                throw CountedPtrError();
            return *_ptr;
        }

        template <typename T_>
        T_ * CountedPtrBase<T_, dereference_policy::CheckedDereferenceTag>::operator-> ()
        {
            if (0 == _ptr)
                throw CountedPtrError();
            return _ptr;
        }
    }

    /**
     * Reference counted pointer class.
     *
     * \ingroup Pointer
     */
    template <typename T_, typename CountPolicy_ = count_policy::InternalCountTag,
             typename DereferencePolicy_ = dereference_policy::UncheckedDereferenceTag>
    class CountedPtr;

    /**
     * Base for an internal counted class.
     *
     * \ingroup Pointer
     */
    template <typename T_, typename DereferencePolicy_ = dereference_policy::UncheckedDereferenceTag>
    class InternalCounted;

    /**
     * Reference counted pointer class (specialisation for ExternalCountTag).
     *
     * \ingroup Pointer
     */
    template <typename T_, typename DereferencePolicy_>
    class CountedPtr<T_, count_policy::ExternalCountTag, DereferencePolicy_> :
        public counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>
    {
        private:
            unsigned * _ref_count;

        public:
            /**
             * Constructor, from a raw pointer.
             */
            explicit CountedPtr(T_ * const ptr) :
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(ptr),
                _ref_count(new unsigned(1))
            {
            }

            /**
             * Constructor, from another CountedPtr.
             */
            CountedPtr(const CountedPtr & other) :
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(other.raw_pointer()),
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
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(other._ptr),
                _ref_count(other.reference_count_pointer())
            {
                ++*_ref_count;
            }

            /**
             * Destructor.
             */
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
             * Fetch our reference count pointer.
             */
            unsigned * reference_count_pointer() const
            {
                return _ref_count;
            }
    };

    /**
     * Reference counted pointer class (specialisation for InternalCountTag).
     *
     * \ingroup Pointer
     */
    template <typename T_, typename DereferencePolicy_>
    class CountedPtr<T_, count_policy::InternalCountTag, DereferencePolicy_> :
        public counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>
    {
        public:
            /**
             * Constructor, from a raw pointer.
             */
            explicit CountedPtr(T_ * const ptr) :
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(ptr)
            {
                if (0 != this->_ptr)
                    ++*this->_ptr->reference_count_pointer();
            }

            /**
             * Constructor, from another CountedPtr.
             */
            CountedPtr(const CountedPtr & other) :
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(other._ptr)
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
                counted_ptr_internals::CountedPtrBase<T_, DereferencePolicy_>(other.raw_pointer())
            {
                if (0 != this->_ptr)
                    ++*this->_ptr->reference_count_pointer();
            }

            /**
             * Destructor.
             */
            ~CountedPtr()
            {
                if (0 != this->_ptr)
                    if (0 == --(*this->_ptr->reference_count_pointer()))
                        delete this->_ptr;
            }

            /**
             * Assignment, from another CountedPtr.
             */
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
             * Fetch our reference count pointer.
             */
            unsigned * reference_count_pointer() const
            {
                if (0 != this->_ptr)
                    return this->_ptr->reference_count_pointer();
                else
                    return 0;
            }
    };
}

#include <paludis/instantiation_policy.hh>

namespace paludis
{
    /**
     * Base class for an internally counted class.
     *
     * \ingroup Pointer
     */
    template <typename T_, typename DereferencePolicy_>
    class InternalCounted :
        private InstantiationPolicy<InternalCounted<T_, DereferencePolicy_>,
            instantiation_method::NonCopyableTag>
    {
        private:
            mutable unsigned _ref_count;

        protected:
            /**
             * Constructor.
             */
            InternalCounted() :
                _ref_count(0)
            {
            }

            /**
             * Destructor.
             */
            ~InternalCounted()
            {
            }

        public:
            /**
             * A CountedPtr to us.
             */
            typedef CountedPtr<T_, count_policy::InternalCountTag, DereferencePolicy_> Pointer;

            /**
             * A CountedPtr to us (const).
             */
            typedef CountedPtr<const T_, count_policy::InternalCountTag, DereferencePolicy_> ConstPointer;

            /**
             * Fetch a pointer to our reference count (may be zero).
             */
            unsigned * reference_count_pointer() const
            {
                return & _ref_count;
            }
    };
}

#endif

