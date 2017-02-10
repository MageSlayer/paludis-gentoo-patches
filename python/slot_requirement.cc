/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include <python/paludis_python.hh>
#include <python/exception.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/sequence.hh>
#include <paludis/slot_requirement.hh>
#include <paludis/name.hh>
#include <memory>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

namespace
{
    class SlotRequirementSptrToPythonVisitor
    {
        private:
            const std::shared_ptr<const SlotRequirement> & _m_ptr;

        public:
            boost::python::object obj;

            SlotRequirementSptrToPythonVisitor(const std::shared_ptr<const SlotRequirement> & m_ptr) :
                _m_ptr(m_ptr)
            {
            }

            void visit(const SlotExactPartialRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotExactPartialRequirement>(_m_ptr));
            }

            void visit(const SlotExactFullRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotExactFullRequirement>(_m_ptr));
            }

            void visit(const SlotAnyAtAllLockedRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotAnyAtAllLockedRequirement>(_m_ptr));
            }

            void visit(const SlotAnyPartialLockedRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotAnyPartialLockedRequirement>(_m_ptr));
            }

            void visit(const SlotAnyUnlockedRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotAnyUnlockedRequirement>(_m_ptr));
            }

            void visit(const SlotUnknownRewrittenRequirement &)
            {
                obj = bp::object(std::static_pointer_cast<const SlotUnknownRewrittenRequirement>(_m_ptr));
            }
    };

    struct SlotRequirementSptrToPython
    {
        SlotRequirementSptrToPython()
        {
            bp::to_python_converter<std::shared_ptr<const SlotRequirement>, SlotRequirementSptrToPython>();
        }

        static PyObject *
        convert(const std::shared_ptr<const SlotRequirement> & m)
        {
            if (! m)
                return Py_None;

            SlotRequirementSptrToPythonVisitor v(m);
            m->accept(v);
            return bp::incref(v.obj.ptr());
        }
    };
}

void expose_slot_requirement()
{
    SlotRequirementSptrToPython();

    /**
     * SlotRequirement
     */
    bp::class_<SlotRequirement, boost::noncopyable>
        (
         "SlotRequirement",
         "A SlotRequirement represents a PackageDepSpec's slot requirement, "
         "such as ``:3``, ``:*``, ``:=`` or ``:=3``.",
         bp::no_init
        )

        .def("__str__", &SlotRequirement::as_string)
        ;

    /**
     * SlotExactPartialRequirement
     */
    register_shared_ptrs_to_python<SlotExactPartialRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotExactPartialRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotExactPartialRequirement, std::shared_ptr<SlotExactPartialRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotExactPartialRequirement",
         "A SlotExactPartialRequirement is a SlotRequirement for exact slot requirements, "
         "such as ``:3`` or ``:=3``.",
         bp::no_init
        )

        .add_property("slot", &SlotExactPartialRequirement::slot,
                "[ro] SlotName\n"
                "The slot in question."
                )
        ;

    /**
     * SlotExactFullRequirement
     */
    register_shared_ptrs_to_python<SlotExactFullRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotExactFullRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::to_python_converter<std::pair<SlotName, SlotName>, pair_to_tuple<SlotName, SlotName>>();
    bp::class_<SlotExactFullRequirement, std::shared_ptr<SlotExactFullRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotExactFullRequirement",
         "A SlotExactFullRequirement is a SlotRequirement for exact slot requirements, "
         "such as ``:3/3`` or ``:=3/3``.",
         bp::no_init
        )

        .add_property("slots", &SlotExactFullRequirement::slots,
                "[ro] (SlotName, SlotName)\n"
                "The slot parts in question."
                )
        ;

    /**
     * SlotAnyAtAllLockedRequirement
     */
    register_shared_ptrs_to_python<SlotAnyAtAllLockedRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotAnyAtAllLockedRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotAnyAtAllLockedRequirement, std::shared_ptr<SlotAnyAtAllLockedRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotAnyAtAllLockedRequirement",
         "A SlotAnyAtAllLockedRequirement is a SlotRequirement for ``:=`` slot requirements.",
         bp::no_init
        )
        ;

    /**
     * SlotAnyPartialLockedRequirement
     */
    register_shared_ptrs_to_python<SlotAnyPartialLockedRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotAnyPartialLockedRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotAnyPartialLockedRequirement, std::shared_ptr<SlotAnyPartialLockedRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotAnyPartialLockedRequirement",
         "A SlotAnyPartialLockedRequirement is a SlotRequirement for ``:3=`` slot requirements.",
         bp::no_init
        )

        .add_property("slot", &SlotAnyPartialLockedRequirement::slot,
                "[ro] SlotName\n"
                "The slot in question."
                )
        ;

    /**
     * SlotAnyUnlockedRequirement
     */
    register_shared_ptrs_to_python<SlotAnyUnlockedRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotAnyUnlockedRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotAnyUnlockedRequirement, std::shared_ptr<SlotAnyUnlockedRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotAnyUnlockedRequirement",
         "A SlotAnyUnlockedRequirement is a SlotRequirement for ``:*`` slot requirements.",
         bp::no_init
        )
        ;

    /**
     * SlotUnknownRewrittenRequirement
     */
    register_shared_ptrs_to_python<SlotUnknownRewrittenRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotUnknownRewrittenRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotUnknownRewrittenRequirement, std::shared_ptr<SlotUnknownRewrittenRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotUnknownRewrittenRequirement",
         "A SlotUnknownRewrittenRequirement is a SlotRequirement for the "
         "maybe_original_requirement_if_written() method of ``:3/3.1= slot`` "
         "requirements that do not indicate whether they were originally "
         "of ``:=`` or ``:3=`` type.",
         bp::no_init
        )
        ;
}

