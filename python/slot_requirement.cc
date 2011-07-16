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
#include <paludis/util/make_null_shared_ptr.hh>
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

            void visit(const SlotExactRequirement & k)
            {
                obj = bp::object(std::static_pointer_cast<const SlotExactRequirement>(_m_ptr));
            }

            void visit(const SlotAnyLockedRequirement & k)
            {
                obj = bp::object(std::static_pointer_cast<const SlotAnyLockedRequirement>(_m_ptr));
            }

            void visit(const SlotAnyUnlockedRequirement & k)
            {
                obj = bp::object(std::static_pointer_cast<const SlotAnyUnlockedRequirement>(_m_ptr));
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
         "such as :3 , :* , := or :=3 .",
         bp::no_init
        )

        .def("__str__", &SlotRequirement::as_string)
        ;

    /**
     * SlotExactRequirement
     */
    register_shared_ptrs_to_python<SlotExactRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotExactRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotExactRequirement, std::shared_ptr<SlotExactRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotExactRequirement",
         "A SlotExactRequirement is a SlotRequirement for exact slot requirements, "
         "such as :3 or :=3 .",
         bp::no_init
        )

        .add_property("slot", &SlotExactRequirement::slot,
                "[ro] SlotName\n"
                "The slot in question."
                )

        .add_property("from_any_locked", &SlotExactRequirement::from_any_locked,
                "[ro] bool\n"
                "If true, indicates we are a :=3 style dependency."
                )
        ;

    /**
     * SlotAnyLockedRequirement
     */
    register_shared_ptrs_to_python<SlotAnyLockedRequirement>(rsp_const);
    bp::implicitly_convertible<std::shared_ptr<SlotAnyLockedRequirement>, std::shared_ptr<SlotRequirement> >();
    bp::class_<SlotAnyLockedRequirement, std::shared_ptr<SlotAnyLockedRequirement>, bp::bases<SlotRequirement>, boost::noncopyable>
        (
         "SlotAnyLockedRequirement",
         "A SlotAnyLockedRequirement is a SlotRequirement for := slot requirements.",
         bp::no_init
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
         "A SlotAnyUnlockedRequirement is a SlotRequirement for :* slot requirements.",
         bp::no_init
        )
        ;
}

