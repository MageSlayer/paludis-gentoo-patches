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

#include "metadata_key.hh"
#include <python/paludis_python.hh>

#include <paludis/name.hh>
#include <paludis/util/visitor-impl.hh>

#include <datetime.h>

using namespace paludis;
using namespace paludis::python;
namespace bp = boost::python;

struct MetadataTimeKeyWrapper
{
    static PyObject *
    value(const MetadataTimeKey & self)
    {
        PyDateTime_IMPORT;
        return PyDateTime_FromTimestamp(bp::make_tuple(self.value()).ptr());
    }

};

template <typename K_>
struct class_set_key :
    bp::class_<K_, bp::bases<MetadataKey>, boost::noncopyable>
{
    class_set_key(const std::string & set) :
        bp::class_<K_, bp::bases<MetadataKey>, boost::noncopyable>(("Metadata" + set + "Key").c_str(), bp::no_init)
    {
        add_property("value", &K_::value,
                ("[ro] " + set + "\n").c_str());
    }
};

template <typename K_>
struct class_spec_tree_key :
    bp::class_<K_, bp::bases<MetadataKey>, boost::noncopyable>
{
    class_spec_tree_key(const std::string & spec_tree) :
        bp::class_<K_, bp::bases<MetadataKey>, boost::noncopyable>(("Metadata" + spec_tree + "Key").c_str(), bp::no_init)
    {
        add_property("value", &K_::value,
                ("[ro] " + spec_tree + "\n").c_str());
    }
};

void
MetadataKeyToPython::visit(const MetadataPackageIDKey & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataStringKey & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataTimeKey & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataContentsKey & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataRepositoryMaskInfoKey & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSetKey<KeywordNameSet> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSetKey<UseFlagNameSet> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSetKey<IUseFlagSet> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSetKey<InheritedSet> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSpecTreeKey<URISpecTree> & k)
{
    value = bp::object(bp::ptr(&k));
}

void
MetadataKeyToPython::visit(const MetadataSetKey<PackageIDSequence> & k)
{
    value = bp::object(bp::ptr(&k));
}

struct metadata_key_to_python
{
    static PyObject *
    convert(const MetadataKey & k)
    {
        MetadataKeyToPython v;
        k.accept(v);
        return bp::incref(v.value.ptr());
    }
};

void register_metadata_key_to_python()
{
    bp::to_python_converter<MetadataKey, metadata_key_to_python>();
}


void expose_metadata_key() PALUDIS_VISIBLE
{
    /**
     * Enums
     */
    enum_auto("MetadataKeyType", last_mkt,
            "The significance of a MetadataKey to a user.");

    /**
     * MetadataKey
     */
    register_shared_ptrs_to_python<MetadataKey>();
    bp::class_<MetadataKey, boost::noncopyable>
        (
         "MetadataKey",
         bp::no_init
        )
        .add_property("raw_name", &MetadataKey::raw_name,
                "[ro] string\n"
                )

        .add_property("human_name", &MetadataKey::human_name,
                "[ro] string\n"
                )
        ;
    register_metadata_key_to_python();

    /**
     * MetadataPackageIDKey
     */
    bp::class_<MetadataPackageIDKey, bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataPackageIDKey",
         bp::no_init
        )
        .add_property("value", &MetadataPackageIDKey::value,
                "[ro] PackageID\n"
                )
        ;

    /**
     * MetadataStringKey
     */
    bp::class_<MetadataStringKey, bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataStringKey",
         bp::no_init
        )
        .add_property("value", &MetadataStringKey::value,
                "[ro] string\n"
                )
        ;

    /**
     * MetadataTimeKey
     */
    bp::class_<MetadataTimeKey, bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataTimeKey",
         bp::no_init
        )
        .add_property("value", &MetadataTimeKeyWrapper::value,
                "[ro] datetime\n"
                )
        ;

    /**
     * MetadataContentsKey
     */
    bp::class_<MetadataContentsKey, bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataContentsKey",
         bp::no_init
        )
        .add_property("value", &MetadataContentsKey::value,
                "[ro] Contents\n"
                )

        //Work around epydoc bug
        .add_property("raw_name", &MetadataContentsKey::raw_name,
                "[ro] string\n"
                )

        //Work around epydoc bug
        .add_property("human_name", &MetadataContentsKey::human_name,
                "[ro] string\n"
                )
        ;

    /**
     * MetadataRepositoryMaskInfoKey
     */
    bp::class_<MetadataRepositoryMaskInfoKey, bp::bases<MetadataKey>, boost::noncopyable>
        (
         "MetadataRepositoryMaskInfoKey",
         bp::no_init
        )
        .add_property("value", &MetadataRepositoryMaskInfoKey::value,
                "[ro] RepositoryMaskInfo\n"
                )
        ;

    /**
     * MetadataSetKeys
     */
    class_set_key<MetadataSetKey<KeywordNameSet> >("KeywordNameIterable");
    class_set_key<MetadataSetKey<UseFlagNameSet> >("UseFlagNameIterable");
    class_set_key<MetadataSetKey<IUseFlagSet> >("IUseFlagIterable");
    class_set_key<MetadataSetKey<InheritedSet> >("InheritedIterable");

    /**
     * MetadataSpecTreeKeys
     */
    class_spec_tree_key<MetadataSpecTreeKey<LicenseSpecTree> >("LicenseSpecTree");
    class_spec_tree_key<MetadataSpecTreeKey<ProvideSpecTree> >("ProvideSpecTree");
    class_spec_tree_key<MetadataSpecTreeKey<DependencySpecTree> >("DependencySpecTree");
    class_spec_tree_key<MetadataSpecTreeKey<RestrictSpecTree> >("RestrictSpecTree");
    class_spec_tree_key<MetadataSpecTreeKey<URISpecTree> >("URISpecTree");
}
