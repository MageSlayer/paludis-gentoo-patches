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

#include <paludis_python.hh>

#include <paludis/qa.hh>
#include <paludis/util/options.hh>
#include <paludis/util/fs_entry.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct QAReporterWrapper :
    QAReporter,
    bp::wrapper<QAReporter>
{
    void message(const QAMessage & msg)
    {
        if (get_override("message"))
            get_override("message")(msg);
        else
            throw PythonMethodNotImplemented("QAReporter", "message");
    }
};

void PALUDIS_VISIBLE expose_qa()
{
    /**
     * Enums
     */
    enum_auto("QAMessageLevel", last_qaml,
            "The importance of a QA notice.");
    enum_auto("QACheckProperty", last_qacp,
            "Properties of a QA check.");

    /**
     * Options
     */
    class_options<QACheckProperties>("QACheckProperties", "QACheckProperty",
            "NEED_DOC");

    /**
     * QAMessage
     */
    bp::class_<QAMessage>
        (
         "QAMessage",
         "NEED_DOC",
         bp::init<const FSEntry &, const QAMessageLevel, const std::string &, const std::string &>()
        )
        .add_property("entry", bp::make_getter(&QAMessage::entry, bp::return_value_policy<bp::return_by_value>()))

        .def_readonly("level", &QAMessage::level)

        .def_readonly("name", &QAMessage::name)

        .def_readonly("message", &QAMessage::message)
        ;

    /**
     * QAReporter
     */
    bp::class_<QAReporterWrapper, boost::noncopyable>
        (
         "QAReporter",
         "NEED_DOC",
         bp::init<>()
        )
        .def("message", bp::pure_virtual(&QAReporter::message),
                "message(QAMessage)\n"
                "NEED_DOC"
            )
        ;
}
