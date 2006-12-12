/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_LIBXML_UTILS_HH
#define PALUDIS_GUARD_PALUDIS_QA_LIBXML_UTILS_HH 1

#include <libxml/parser.h>
#include <paludis/util/attributes.hh>
#include <string>

namespace paludis
{
    /**
     * Wrapper around libxml pointers.
     *
     * RAII, the specified function is used to free the pointer.
     *
     * \ingroup grpxml
     */
    template <typename PtrType_>
    class LibXmlPtrHolder
    {
        private:
            PtrType_ _ptr;
            void (* _free_func) (PtrType_);

            LibXmlPtrHolder(const LibXmlPtrHolder &);
            void operator= (const LibXmlPtrHolder &);

        public:
            ///\name Basic operations
            ///\{

            LibXmlPtrHolder(PtrType_ ptr, void (* free_func) (PtrType_)) :
                _ptr(ptr),
                _free_func(free_func)
            {
            }

            ~LibXmlPtrHolder()
            {
                if (0 != _ptr)
                    _free_func(_ptr);
            }

            ///\}

            /**
             * Cast to our pointer type.
             */
            operator PtrType_ () const
            {
                return _ptr;
            }
    };

    /**
     * Turn a retarded libxml string into a std::string.
     *
     * \ingroup grpxml
     */
    std::string
    retarded_libxml_string_to_string(const xmlChar * s) PALUDIS_VISIBLE;

    /**
     * Remove leading and trailing whitespace, and normalise internal whitespace.
     *
     * \ingroup grpxml
     */
    std::string normalise(const std::string & s) PALUDIS_VISIBLE;
}


#endif
