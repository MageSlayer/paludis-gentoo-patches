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

#include <python/paludis_python.hh>
#include <python/exception.hh>
#include <python/iterable.hh>

#include <paludis/metadata_key.hh>
#include <paludis/name.hh>
#include <paludis/dep_label.hh>
#include <paludis/environment.hh>
#include <paludis/maintainer.hh>
#include <paludis/slot.hh>
#include <paludis/choice.hh>
#include <paludis/package_id.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/timestamp.hh>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

class MetadataKeySptrToPythonVisitor
{
    private:
        const std::shared_ptr<const MetadataKey> & _m_ptr;

    public:
        boost::python::object obj;

        MetadataKeySptrToPythonVisitor(const std::shared_ptr<const MetadataKey> & m_ptr) :
            _m_ptr(m_ptr)
        {
        }

        void visit(const MetadataValueKey<std::shared_ptr<const PackageID> > &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<std::shared_ptr<const PackageID> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<std::string> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<std::string> >(_m_ptr));
        }

        void visit(const MetadataValueKey<Slot> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<Slot> >(_m_ptr));
        }

        void visit(const MetadataValueKey<long> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<long> >(_m_ptr));
        }

        void visit(const MetadataValueKey<bool> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<bool> >(_m_ptr));
        }

        void visit(const MetadataTimeKey &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataTimeKey>(_m_ptr));
        }

        void visit(const MetadataValueKey<std::shared_ptr<const Choices> > &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<std::shared_ptr<const Choices> > >(_m_ptr));
        }

        void visit(const MetadataValueKey<FSPath> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataValueKey<FSPath> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<KeywordNameSet> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<KeywordNameSet> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Set<std::string> > &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<Set<std::string> > >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Map<std::string, std::string> > &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<Map<std::string, std::string> > >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Sequence<std::string> > &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<Sequence<std::string> > >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<FSPathSequence> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<FSPathSequence> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<Maintainers> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<Maintainers> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<LicenseSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<DependencySpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<PlainTextSpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<PlainTextSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<RequiredUseSpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<RequiredUseSpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<FetchableURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSpecTreeKey<SimpleURISpecTree> >(_m_ptr));
        }

        void visit(const MetadataCollectionKey<PackageIDSequence> &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataCollectionKey<PackageIDSequence> >(_m_ptr));
        }

        void visit(const MetadataSectionKey &)
        {
            obj = bp::object(std::static_pointer_cast<const MetadataSectionKey>(_m_ptr));
        }
};

struct MetadataKeySptrToPython
{
    MetadataKeySptrToPython()
    {
        bp::to_python_converter<std::shared_ptr<const MetadataKey>, MetadataKeySptrToPython>();
    }

    static PyObject *
    convert(const std::shared_ptr<const MetadataKey> & m)
    {
        MetadataKeySptrToPythonVisitor v(m);
        m->accept(v);
        return bp::incref(v.obj.ptr());
    }
};

struct MetadataPackageIDKeyWrapper :
    MetadataValueKey<std::shared_ptr<const PackageID> > ,
    bp::wrapper<MetadataValueKey<std::shared_ptr<const PackageID> > >
{
    const std::shared_ptr<const PackageID> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataPackageIDKey", "type");
    }

    const std::string pretty_print_value(
            const PrettyPrinter &,
            const PrettyPrintOptions &) const override
    {
        throw PythonMethodNotImplemented("MetadataPackageIDKey", "pretty_print_value");
    }
};

struct MetadataStringKeyWrapper :
    MetadataValueKey<std::string> ,
    bp::wrapper<MetadataValueKey<std::string> >
{
    const std::string parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataStringKey", "type");
    }
};

struct MetadataSlotNameKeyWrapper :
    MetadataValueKey<Slot> ,
    bp::wrapper<MetadataValueKey<Slot> >
{
    const Slot parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSlotNameKey", "type");
    }
};

struct MetadataSectionKeyWrapper :
    MetadataSectionKey,
    bp::wrapper<MetadataSectionKey>
{
    static PyObject *
    find_metadata(const MetadataSectionKey & self, const std::string & key)
    {
        MetadataSectionKey::MetadataConstIterator i(self.find_metadata(key));
        if (i != self.end_metadata())
            return bp::incref(bp::object(*i).ptr());
        else
            return Py_None;
    }

    void need_keys_added() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("need_keys_added"))
            f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "need_keys_added");
    }

    virtual const std::shared_ptr<const MetadataValueKey<std::string> > title_key() const
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("title_key"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "title_key");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSectionKey", "type");
    }
};

struct MetadataTimeKeyWrapper :
    MetadataTimeKey,
    bp::wrapper<MetadataTimeKey>
{
    Timestamp parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return Timestamp(f(), 0);
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataTimeKey", "type");
    }
};

struct MetadataChoicesKeyWrapper :
    MetadataValueKey<std::shared_ptr<const Choices> > ,
    bp::wrapper<MetadataValueKey<std::shared_ptr<const Choices> > >
{
    const std::shared_ptr<const Choices> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataChoicesKey", "type");
    }
};

struct MetadataFSPathKeyWrapper :
    MetadataValueKey<FSPath> ,
    bp::wrapper<MetadataValueKey<FSPath> >
{
    const FSPath parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataValueKey<FSPath> ", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSPathKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSPathKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataFSPathKey", "type");
    }
};

template <typename C_>
struct MetadataCollectionKeyWrapper :
    MetadataCollectionKey<C_>,
    bp::wrapper<MetadataCollectionKey<C_> >
{
    const std::shared_ptr<const C_> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataCollectionKey", "type");
    }

    const std::string pretty_print_value(
            const PrettyPrinter &,
            const PrettyPrintOptions &) const override
    {
        throw PythonMethodNotImplemented("MetadataCollectionKey", "pretty_print_value");
    }
};

template <typename C_>
struct MetadataSpecTreeKeyWrapper :
    MetadataSpecTreeKey<C_>,
    bp::wrapper<MetadataSpecTreeKey<C_> >
{
    const std::shared_ptr<const C_> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "parse_value");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }

    const std::string pretty_print_value(
            const PrettyPrinter &,
            const PrettyPrintOptions &) const override
    {
        throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_value");
    }
};

template <>
struct MetadataSpecTreeKeyWrapper<FetchableURISpecTree> :
    MetadataSpecTreeKey<FetchableURISpecTree>,
    bp::wrapper<MetadataSpecTreeKey<FetchableURISpecTree> >
{
    const std::shared_ptr<const FetchableURISpecTree> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "parse_value");
    }

    const std::shared_ptr<const URILabel> initial_label() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("initial_label"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "initial_label");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }

    const std::string pretty_print_value(
            const PrettyPrinter &,
            const PrettyPrintOptions &) const override
    {
        throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_value");
    }
};

template <>
struct MetadataSpecTreeKeyWrapper<DependencySpecTree> :
    MetadataSpecTreeKey<DependencySpecTree>,
    bp::wrapper<MetadataSpecTreeKey<DependencySpecTree> >
{
    const std::shared_ptr<const DependencySpecTree> parse_value() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("parse_value"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "parse_value");
    }

    const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
        override PALUDIS_ATTRIBUTE((warn_unused_result))
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = this->get_override("initial_labels"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "initial_labels");
    }

    const std::string raw_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("raw_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "raw_name");
    }

    const std::string human_name() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("human_name"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "human_name");
    }

    MetadataKeyType type() const override
    {
        std::unique_lock<std::recursive_mutex> l(get_mutex());

        if (bp::override f = get_override("type"))
            return f();
        else
            throw PythonMethodNotImplemented("MetadataSpecTreeKey", "type");
    }

    const std::string pretty_print_value(
            const PrettyPrinter &,
            const PrettyPrintOptions &) const override
    {
        throw PythonMethodNotImplemented("MetadataSpecTreeKey", "pretty_print_value");
    }
};

template <typename C_>
struct class_set_key :
    bp::class_<MetadataCollectionKeyWrapper<C_>, std::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_set_key(const std::string & set, const std::string & cn) :
        bp::class_<MetadataCollectionKeyWrapper<C_>, std::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + cn + "Key").c_str(),
                    "A MetadataCollectionKey is a MetadataKey that holds a Set or Sequence of some kind of item\n"
                    "as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::shared_ptr<const MetadataCollectionKey<C_> > >();
        bp::implicitly_convertible<std::shared_ptr<MetadataCollectionKeyWrapper<C_> >,
                std::shared_ptr<MetadataKey> >();

        this->def("parse_value", bp::pure_virtual(&MetadataCollectionKey<C_>::parse_value),
                ("parse_value() -> " + set + "\n"
                 "Fetch our value.").c_str()
           );
    }
};

template <typename C_>
struct class_spec_tree_key :
    bp::class_<MetadataSpecTreeKeyWrapper<C_>, std::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<C_>, std::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::shared_ptr<const MetadataSpecTreeKey<C_> > >();
        bp::implicitly_convertible<std::shared_ptr<MetadataSpecTreeKeyWrapper<C_> >,
                std::shared_ptr<MetadataKey> >();

        this->def("parse_value", bp::pure_virtual(&MetadataSpecTreeKey<C_>::parse_value),
                ("parse_value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );
    }
};

template <>
struct class_spec_tree_key<FetchableURISpecTree> :
    bp::class_<MetadataSpecTreeKeyWrapper<FetchableURISpecTree>,
        std::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<FetchableURISpecTree>, 
            std::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::shared_ptr<const MetadataSpecTreeKey<FetchableURISpecTree> > >();
        bp::implicitly_convertible<std::shared_ptr<MetadataSpecTreeKeyWrapper<FetchableURISpecTree> >,
                std::shared_ptr<MetadataKey> >();

        this->def("parse_value", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::parse_value),
                ("parse_value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );

        this->def("initial_label", bp::pure_virtual(&MetadataSpecTreeKey<FetchableURISpecTree>::initial_label),
                "initial_label() -> URILabel\n"
                "Return a URILabel that represents the initial label to use when\n"
                "deciding the behaviour of individual items in the heirarchy."
           );
    }
};

template <>
struct class_spec_tree_key<DependencySpecTree> :
    bp::class_<MetadataSpecTreeKeyWrapper<DependencySpecTree>,
        std::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
        bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<MetadataSpecTreeKeyWrapper<DependencySpecTree>, 
            std::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
            bp::bases<MetadataKey>, boost::noncopyable>(
                    ("Metadata" + spec_tree + "Key").c_str(),
                    "A MetadataSpecTreeKey is a MetadataKey that holds a spec tree of some\n"
                    "kind as its value.\n\n"

                    "This class can be subclassed in Python.",
                    bp::init<>(
                        "__init__()"
                        )
                    )
    {
        bp::register_ptr_to_python<std::shared_ptr<const MetadataSpecTreeKey<DependencySpecTree> > >();
        bp::implicitly_convertible<std::shared_ptr<MetadataSpecTreeKeyWrapper<DependencySpecTree> >,
                std::shared_ptr<MetadataKey> >();

        this->def("parse_value", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::parse_value),
                ("parse_value() -> " + spec_tree + "\n"
                 "Fetch our value").c_str()
           );

        this->def("initial_labels", bp::pure_virtual(&MetadataSpecTreeKey<DependencySpecTree>::initial_labels),
                "initial_label() -> DependenciesLabelSequence\n"
                "Return a DependenciesLabelSequence that represents the initial labels to use when\n"
                "deciding the behaviour of individual items in the heirarchy."
           );
    }
};

void expose_metadata_key()
{
    /**
     * Enums
     */
    enum_auto("MetadataKeyType", last_mkt,
            "The significance of a MetadataKey to a user.");

    /**
     * MetadataKey
     */
    MetadataKeySptrToPython();
    register_shared_ptrs_to_python<MetadataKey>(rsp_non_const);
    bp::class_<MetadataKey, boost::noncopyable>
        (
         "MetadataKey",
         "A MetadataKey is a generic key that contains a particular piece of\n"
         "information about a PackageID instance.\n\n"

         "A basic MetadataKey has:\n\n"

         "- A raw name. This is in a repository-defined format designed to closely\n"
         "  represent the internal name. For example, ebuilds and VDB IDs use\n"
         "  raw names like ``DESCRIPTION`` and ``KEYWORDS``, whereas CRAN uses names\n"
         "  like ``Title`` and ``BundleDescription``. The raw name is unique in a\n"
         "  PackageID.\n\n"

         "- A human name. This is the name that should be used when outputting\n"
         "  normally for a human to read.\n\n"

         "- A MetadataKeyType. This is a hint to clients as to whether the key\n"
         "  should be displayed when outputting information about a package ID.\n\n"

         "Subclasses provide additional information, including the 'value' of the\n"
         "key.",
         bp::no_init
        )
        .def("raw_name", bp::pure_virtual(&MetadataKey::raw_name),
                "raw_name() -> string\n"
                "Fetch our raw name."
                )

        .def("human_name", bp::pure_virtual(&MetadataKey::human_name),
                "human_name() -> string\n"
                "Fetch our human name."
                )

        .def("type", bp::pure_virtual(&MetadataKey::type),
                "type() -> MetadataKeyType\n"
                "Fetch our key type."
                )
        ;

    /**
     * MetadataPackageIDKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataValueKey<std::shared_ptr<const PackageID> > > >();
    bp::implicitly_convertible<std::shared_ptr<MetadataPackageIDKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataPackageIDKeyWrapper, std::shared_ptr<MetadataPackageIDKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataPackageIDKey",
         "A MetadataPackageIDKey is a MetadataKey that has a PackageID as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataValueKey<std::shared_ptr<const PackageID> > ::parse_value),
                "parse_value() -> PackageID\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataStringKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataValueKey<std::string> > >();
    bp::implicitly_convertible<std::shared_ptr<MetadataStringKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataStringKeyWrapper, std::shared_ptr<MetadataStringKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataStringKey",
         "A MetadataStringKey is a MetadataKey that has a std::string as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataValueKey<std::string> ::parse_value),
                "parse_value() -> string\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSlotNameKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataValueKey<Slot> > >();
    bp::implicitly_convertible<std::shared_ptr<MetadataSlotNameKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataSlotNameKeyWrapper, std::shared_ptr<MetadataSlotNameKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataSlotNameKey",
         "A MetadataStringKey is a MetadataKey that has a Slot as its\n"
         "value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataValueKey<Slot> ::parse_value),
                "parse_value() -> Slot\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataSectionKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataSectionKey> >();
    bp::implicitly_convertible<std::shared_ptr<MetadataSectionKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataSectionKeyWrapper, std::shared_ptr<MetadataSectionKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataSectionKey",
         "A MetadataSectionKey holds a number of other MetadataKey instances, and\n"
         "may have a title.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .add_property("metadata", bp::range(&MetadataSectionKey::begin_metadata, &MetadataSectionKey::end_metadata),
                "[ro] Iterable of MetadataKey\n"
                "NEED_DOC"
                )

        .def("find_metadata", &MetadataSectionKeyWrapper::find_metadata,
                "find_metadata(string) -> MetadataKey\n"
                "NEED_DOC"
            )
        ;

    /**
     * MetadataTimeKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataTimeKey> >();
    bp::implicitly_convertible<std::shared_ptr<MetadataTimeKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataTimeKeyWrapper, std::shared_ptr<MetadataTimeKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataTimeKey",
         "A MetadataTimeKey is a MetadataKey that has a ``int(time_t)`` as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataTimeKey::parse_value),
                "parse_value() -> int\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataFSPathKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataValueKey<FSPath> > >();
    bp::implicitly_convertible<std::shared_ptr<MetadataFSPathKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataFSPathKeyWrapper, std::shared_ptr<MetadataFSPathKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataFSPathKey",
         "A MetadataFSPathKey is a MetadataKey that has an ``string(FSPath)`` as its value.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataValueKey<FSPath> ::parse_value),
                "parse_value() -> FSPath\n"
                "Fetch our value."
                )
        ;

    /**
     * MetadataChoicesKey
     */
    bp::register_ptr_to_python<std::shared_ptr<const MetadataValueKey<std::shared_ptr<const Choices> > > >();
    bp::implicitly_convertible<std::shared_ptr<MetadataChoicesKeyWrapper>,
            std::shared_ptr<MetadataKey> >();
    bp::class_<MetadataChoicesKeyWrapper, std::shared_ptr<MetadataChoicesKeyWrapper>,
            bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataChoicesKey",
         "A MetadataChoicesKey is a MetadataKey that holds a Choices heirarchy.\n\n"

         "This class can be subclassed in Python.",
         bp::init<>(
             "__init__()"
             )
        )
        .def("parse_value", bp::pure_virtual(&MetadataValueKey<std::shared_ptr<const Choices> > ::parse_value),
                "parse_value() -> Choices\n"
                "Fetch our value."
                )

        //Work around epydoc bug
        .def("raw_name", bp::pure_virtual(&MetadataKey::raw_name),
                "raw_name() -> string\n"
                "Fetch our raw name."
                )

        //Work around epydoc bug
        .def("human_name", bp::pure_virtual(&MetadataKey::human_name),
                "human_name() -> string\n"
                "Fetch our human name."
                )

        //Work around epydoc bug
        .def("type", bp::pure_virtual(&MetadataKey::type),
                "type() -> MetadataKeyType\n"
                "Fetch our key type."
                )
        ;


    /**
     * MetadataCollectionKeys
     */
    class_set_key<KeywordNameSet>("KeywordNameIterable", "KeywordNameIterable");
    class_set_key<Set<std::string> >("StringIterable", "StringIterable");
    class_set_key<Sequence<std::string> >("StringIterable", "StringSequence");
    class_set_key<FSPathSequence>("FSPathIterable", "FSPathIterable");
    class_set_key<PackageIDSequence>("PackageIDIterable", "PackageIDIterable");
    class_set_key<Maintainers>("MaintainerIterable", "MaintainerIterable");

    /**
     * MetadataSpecTreeKeys
     */
    class_spec_tree_key<LicenseSpecTree>("LicenseSpecTree");
    class_spec_tree_key<DependencySpecTree>("DependencySpecTree");
    class_spec_tree_key<PlainTextSpecTree>("PlainTextSpecTree");
    class_spec_tree_key<RequiredUseSpecTree>("RequiredUseSpecTree");
    class_spec_tree_key<SimpleURISpecTree>("SimpleURISpecTree");
    class_spec_tree_key<FetchableURISpecTree>("FetchableURISpecTree");

    /**
     * Maintainers
     */
    class_iterable<Maintainers>
        (
         "MaintainerIterable",
         "Iterable of Maintainer",
         true
        );

    /**
     * Maintainer
     */
    bp::class_<Maintainer> maintainer(
            "Maintainer",
            "Represents a package maintainer",
            bp::no_init
            );

    maintainer
        .add_property("author",
                &named_values_getter<Maintainer, n::author, std::string, &Maintainer::author>,
                "[ro] String\n"
                )

        .add_property("description",
                &named_values_getter<Maintainer, n::description, std::string, &Maintainer::description>,
                "[ro] String\n"
                )

        .add_property("email",
                &named_values_getter<Maintainer, n::email, std::string, &Maintainer::email>,
                "[ro] String\n"
                )
        ;
}

