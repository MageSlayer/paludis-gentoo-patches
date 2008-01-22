/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
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

#ifndef PALUDIS_GUARD_PYTHON_VALIDATED_HH
#define PALUDIS_GUARD_PYTHON_VALIDATED_HH 1

#include <python/paludis_python.hh>

#include <paludis/util/validated.hh>

namespace paludis
{
    namespace python
    {
        // expose Validated classes
        template <typename V_>
        class class_validated;

        template <typename ValidatedDataType_, typename Validator_, typename Comparator_>
        class class_validated<Validated<ValidatedDataType_, Validator_, true, Comparator_> > :
            public boost::python::class_<Validated<ValidatedDataType_, Validator_, true, Comparator_> >
        {
            public:
                class_validated(const std::string & name,
                        const std::string & class_doc, const std::string & init_arg="string") :
                    boost::python::class_<Validated<ValidatedDataType_, Validator_, true, Comparator_> >(
                            name.c_str(), class_doc.c_str(),
                            boost::python::init<const ValidatedDataType_ &>(("__init__("+init_arg+")").c_str())
                            )
                {
                    this->def(boost::python::self_ns::str(boost::python::self));
                    this->def("__cmp__",
                              &paludis::python::py_cmp<Validated<ValidatedDataType_, Validator_, true, Comparator_> >);
                    boost::python::implicitly_convertible<ValidatedDataType_,
                            Validated<ValidatedDataType_, Validator_, true, Comparator_> >();
                }
        };

        template <typename ValidatedDataType_, typename Validator_, typename Comparator_>
        class class_validated<Validated<ValidatedDataType_, Validator_, false, Comparator_> > :
            public boost::python::class_<Validated<ValidatedDataType_, Validator_, false, Comparator_> >
        {
            public:
                class_validated(const std::string & name,
                        const std::string & class_doc, const std::string & init_arg="string") :
                    boost::python::class_<Validated<ValidatedDataType_, Validator_, false, Comparator_> >(
                            name.c_str(), class_doc.c_str(),
                            boost::python::init<const ValidatedDataType_ &>(("__init__("+init_arg+")").c_str())
                            )
                {
                    this->def(boost::python::self_ns::str(boost::python::self));
                    this->def("__eq__",
                            &paludis::python::py_eq<Validated<ValidatedDataType_, Validator_, false, Comparator_> >);
                    this->def("__ne__",
                            &paludis::python::py_ne<Validated<ValidatedDataType_, Validator_, false, Comparator_> >);
                    boost::python::implicitly_convertible<ValidatedDataType_,
                            Validated<ValidatedDataType_, Validator_, false, Comparator_> >();
                }
        };

    } // namespace paludis::python
} // namespace paludis

#endif
