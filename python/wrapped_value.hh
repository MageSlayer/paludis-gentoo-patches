/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
 * Copyright (c) 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PYTHON_WRAPPED_VALUE_HH
#define PALUDIS_GUARD_PYTHON_WRAPPED_VALUE_HH 1

#include <python/paludis_python.hh>
#include <paludis/util/wrapped_value.hh>

namespace paludis
{
    namespace python
    {
        template <typename V_>
        class class_wrapped_value;

        template <typename Tag_>
        class class_wrapped_value<WrappedValue<Tag_> > :
            public boost::python::class_<WrappedValue<Tag_> >
        {
            public:
                class_wrapped_value(const std::string & name,
                        const std::string & class_doc, const std::string & init_arg="string") :
                    boost::python::class_<WrappedValue<Tag_> >(
                            name.c_str(), class_doc.c_str(),
                            boost::python::init<const typename WrappedValueTraits<Tag_>::UnderlyingType &>(("__init__("+init_arg+")").c_str())
                            )
                {
                    this->def(boost::python::self_ns::str(boost::python::self));
                    this->def("__cmp__",
                              &paludis::python::py_cmp<WrappedValue<Tag_> >);
                    boost::python::implicitly_convertible<typename WrappedValueTraits<Tag_>::UnderlyingType,
                            WrappedValue<Tag_> >();
                }
        };
    }
}

#endif
