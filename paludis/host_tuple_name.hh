/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
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

#ifndef PALUDIS_GUARD_PALUDIS_HOST_TUPLE_NAME_HH
#define PALUDIS_GUARD_PALUDIS_HOST_TUPLE_NAME_HH 1

#include <paludis/util/exception.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/validated.hh>

#include <string>
#include <iosfwd>

/** \file
 * Declarations for various additional cross-compiling related Name classes.
 *
 * \ingroup g_names
 *
 * \section Examples
 *
 * - \ref example_name.cc "example_name.cc"
 */

namespace paludis
{
    /**
     * A HostTupleNameError is thrown if an invalid value is assigned to
     * an HostTupleName.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE HostTupleNameError : public NameError
    {
        public:
            ///\name Basic operations
            ///\{

            HostTupleNameError(const std::string & name) throw ();

            HostTupleNameError(const std::string & name,
                    const std::string & type) throw ();

            ///\}
    };

    /**
     * An ArchitectureNamePartError is thrown if an invalid value is assigned to
     * an ArchitectureNamePart.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE ArchitectureNamePartError : public HostTupleNameError
    {
        public:
            /**
             * Constructor.
             */
            ArchitectureNamePartError(const std::string & name) throw ();
    };

    /**
     * An ArchitectureNamePartValidator handles validation rules for the value
     * of an ArchitectureNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE ArchitectureNamePartValidator :
        private InstantiationPolicy<ArchitectureNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for an ArchitectureNamePart,
         * throw an ArchitectureNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * An ArchitectureNamePart holds a std::string that is a valid name for the
     * architecture part of a HostTupleName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, ArchitectureNamePartValidator> ArchitectureNamePart;

    /**
     * An ManufacturerNamePartError is thrown if an invalid value is assigned to
     * an ManufacturerNamePart.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE ManufacturerNamePartError : public HostTupleNameError
    {
        public:
            /**
             * Constructor.
             */
            ManufacturerNamePartError(const std::string & name) throw ();
    };

    /**
     * An ManufacturerNamePartValidator handles validation rules for the value
     * of an ManufacturerNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE ManufacturerNamePartValidator :
        private InstantiationPolicy<ManufacturerNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for an ManufacturerNamePart,
         * throw an ManufacturerNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * An ManufacturerNamePart holds a std::string that is a valid name for the
     * architecture part of a HostTupleName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, ManufacturerNamePartValidator> ManufacturerNamePart;

    /**
     * An KernelNamePartError is thrown if an invalid value is assigned to
     * an KernelNamePart.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE KernelNamePartError : public HostTupleNameError
    {
        public:
            /**
             * Constructor.
             */
            KernelNamePartError(const std::string & name) throw ();
    };

    /**
     * An KernelNamePartValidator handles validation rules for the value
     * of an KernelNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE KernelNamePartValidator :
        private InstantiationPolicy<KernelNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for an KernelNamePart,
         * throw an KernelNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * An KernelNamePart holds a std::string that is a valid name for the
     * architecture part of a HostTupleName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, KernelNamePartValidator> KernelNamePart;

    /**
     * An UserlandNamePartError is thrown if an invalid value is assigned to
     * an UserlandNamePart.
     *
     * \ingroup g_names
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE UserlandNamePartError : public HostTupleNameError
    {
        public:
            /**
             * Constructor.
             */
            UserlandNamePartError(const std::string & name) throw ();
    };

    /**
     * An UserlandNamePartValidator handles validation rules for the value
     * of an UserlandNamePart.
     *
     * \ingroup g_names
     */
    struct PALUDIS_VISIBLE UserlandNamePartValidator :
        private InstantiationPolicy<UserlandNamePartValidator, instantiation_method::NonInstantiableTag>
    {
        /**
         * If the parameter is not a valid value for an UserlandNamePart,
         * throw an UserlandNamePartError.
         */
        static void validate(const std::string &);
    };

    /**
     * An UserlandNamePart holds a std::string that is a valid name for the
     * architecture part of a HostTupleName.
     *
     * \ingroup g_names
     */
    typedef Validated<std::string, UserlandNamePartValidator> UserlandNamePart;

#include <paludis/host_tuple_name-sr.hh>


    /**
     * Output a QualifiedPackageName to a stream.
     *
     * \ingroup g_names
     */
    std::ostream & operator<< (std::ostream &, const HostTupleName &) PALUDIS_VISIBLE;
}

#endif
