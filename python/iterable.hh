/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#ifndef PALUDIS_GUARD_PYTHON_ITERABLE_HH
#define PALUDIS_GUARD_PYTHON_ITERABLE_HH 1

#include <python/paludis_python.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>

namespace paludis
{
    namespace python
    {
        // expose iterable classes
        template <typename C_>
        class class_iterable :
            public boost::python::class_<C_, boost::noncopyable>
        {
            public:
                class_iterable(const std::string & name, const std::string & class_doc) :
                    boost::python::class_<C_, boost::noncopyable>(name.c_str(), class_doc.c_str(),
                            boost::python::no_init)
                {
                    this->def("__iter__", boost::python::range(&C_::begin, &C_::end));
                    register_shared_ptrs_to_python<C_>();
                }
        };
    } // namespace paludis::python
} // namespace paludis

#endif
