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

#include "additional_tests.hh"

#include <python/paludis_python.hh>

#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/indirect_iterator-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/environment.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/mask.hh>
#include <paludis/hook.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/util/options.hh>
#include <paludis/formatter.hh>
#include <paludis/stringify_formatter-impl.hh>
#include <paludis/util/clone-impl.hh>
#include <paludis/util/timestamp.hh>
#include <memory>

using namespace paludis;
namespace bp = boost::python;

namespace environment
{
    void test_env(Environment & e)
    {
        std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                        n::environment() = &e,
                        n::name() = RepositoryName("fakerepo"))));
        std::shared_ptr<PackageID> pid(repo->add_version("cat", "pkg", "1.0"));
        e.package_database()->add_repository(0, repo);

        bool PALUDIS_ATTRIBUTE((unused)) b2(e.accept_license("l", *pid));

        std::shared_ptr<KeywordNameSet> kns(std::make_shared<KeywordNameSet>());
        kns->insert(KeywordName("keyword"));
        bool PALUDIS_ATTRIBUTE((unused)) b3(e.accept_keywords(kns, *pid));

        e.mask_for_breakage(*pid);

        e.mask_for_user(*pid, false);

        bool PALUDIS_ATTRIBUTE((unused)) b4(e.unmasked_by_user(*pid));

        e.package_database();

        e.bashrc_files();

        e.syncers_dirs();

        e.fetchers_dirs();

        e.hook_dirs();

        e.paludis_command();

        e.set_paludis_command("paludis");

        e.root();

        e.reduced_uid();

        e.reduced_gid();

        e.mirrors("mirror");

        e.set_names();

        e.set(SetName("set"));

        e.default_destinations();

        e.perform_hook(Hook("test"));

        e.distribution();

        e.begin_metadata();
    }
}

namespace mask
{
    void test_mask(Mask & m)
    {
        m.key();
        m.description();
    }

    void test_user_mask(UserMask & m)
    {
        test_mask(m);
    }

    void test_unaccepted_mask(UnacceptedMask & m)
    {
        test_mask(m);
        m.unaccepted_key();
    }

    void test_repository_mask(RepositoryMask & m)
    {
        test_mask(m);
        m.mask_key();
    }

    void test_unsupported_mask(UnsupportedMask & m)
    {
        test_mask(m);
        m.explanation();
    }

    void test_association_mask(AssociationMask & m)
    {
        test_mask(m);
        m.associated_package();
    }
}

namespace metadata_key
{
    void test_metadata_key(const MetadataKey & m)
    {
        m.raw_name();
        m.human_name();
        MetadataKeyType foo(m.type());
    }

    void test_metadata_package_id_key(const MetadataValueKey<std::shared_ptr<const PackageID> > & m)
    {
        test_metadata_key(m);
        m.value();
    }

    void test_metadata_string_key(const MetadataValueKey<std::string> & m)
    {
        test_metadata_key(m);
        m.value();
    }

    void test_metadata_section_key(const MetadataSectionKey & m)
    {
        test_metadata_key(m);
        std::for_each(indirect_iterator(m.begin_metadata()), indirect_iterator(m.end_metadata()),
                &test_metadata_key);
    }

    void test_metadata_time_key(const MetadataTimeKey & m)
    {
        test_metadata_key(m);
        Timestamp PALUDIS_ATTRIBUTE((unused)) t(m.value());
    }

    void test_metadata_contents_key(const MetadataValueKey<std::shared_ptr<const Contents> > & m)
    {
        test_metadata_key(m);
        m.value();
    }

    void test_metadata_choices_key(const MetadataValueKey<std::shared_ptr<const Choices> > & m)
    {
        test_metadata_key(m);
        m.value();
    }

    void test_metadata_repository_mask_info_key(const MetadataValueKey<std::shared_ptr<const RepositoryMaskInfo> > & m)
    {
        test_metadata_key(m);
        m.value();
    }

    template <typename C_>
    void test_metadata_set_key(const MetadataCollectionKey<C_> & m)
    {
        test_metadata_key(m);
        m.value();
        StringifyFormatter ff;
        m.pretty_print_flat(ff);
    }

    template <typename C_>
    void test_metadata_spec_tree_key(const MetadataSpecTreeKey<C_> & m)
    {
        test_metadata_key(m);
        m.value();
        StringifyFormatter ff;
        m.pretty_print(ff);
        m.pretty_print_flat(ff);
    }

    template <>
    void test_metadata_spec_tree_key(const MetadataSpecTreeKey<FetchableURISpecTree> & m)
    {
        test_metadata_key(m);
        m.value();
        StringifyFormatter ff;
        m.pretty_print(ff);
        m.pretty_print_flat(ff);
        m.initial_label();
    }
}

namespace formatter
{
    using namespace format;

    // CanFormat for PlainRoles
    void test_plain_roles(CanFormat<PlainTextDepSpec> & f)
    {
        PlainTextDepSpec d("foo");
        f.format(d, Plain());
    }

    // CanFormat for AcceptableRoles
    void test_acceptable_roles(CanFormat<KeywordName> & f)
    {
        KeywordName k("keyword");
        f.format(k, Plain());
        f.format(k, Accepted());
        f.format(k, Unaccepted());
    }

    // CanFormat for PackageRoles
    void test_package_roles(CanFormat<PackageDepSpec> & f)
    {
        TestEnvironment e;
        PackageDepSpec p(parse_user_package_dep_spec("cat/pkg", &e, UserPackageDepSpecOptions()));
        f.format(p, Plain());
        f.format(p, Installed());
        f.format(p, Installable());
    }

    // CanSpace
    void test_can_space(CanSpace & f)
    {
        f.newline();
        f.indent(1);
    }

    void test_formatter_string(const Formatter<std::string> &)
    {
    }

    void test_formmater_keyword_name(const Formatter<KeywordName> &)
    {
    }

    void test_formatter_license_spec_tree(const LicenseSpecTree::ItemFormatter &)
    {
    }

    void test_formatter_provide_spec_tree(const ProvideSpecTree::ItemFormatter &)
    {
    }

    void test_formatter_dependency_spec_tree(const DependencySpecTree::ItemFormatter &)
    {
    }

    void test_formatter_plain_text_spec_tree(const PlainTextSpecTree::ItemFormatter &)
    {
    }

    void test_formatter_simple_uri_spec_tree(const SimpleURISpecTree::ItemFormatter &)
    {
    }

    void test_formatter_fetchable_uri_spec_tree(const FetchableURISpecTree::ItemFormatter &)
    {
    }
}

void expose_additional_tests()
{
    /**
     * Environemnt tests
     */
    bp::def("test_env", &environment::test_env);

    /**
     * Mask tests
     */
    bp::def("test_user_mask", &mask::test_user_mask);
    bp::def("test_unaccepted_mask", &mask::test_unaccepted_mask);
    bp::def("test_repository_mask", &mask::test_repository_mask);
    bp::def("test_unsupported_mask", &mask::test_unsupported_mask);
    bp::def("test_association_mask", &mask::test_association_mask);

    /**
     * MetadataKey tests
     */
    bp::def("test_metadata_package_id_key", &metadata_key::test_metadata_package_id_key);
    bp::def("test_metadata_string_key", &metadata_key::test_metadata_string_key);
    bp::def("test_metadata_time_key", &metadata_key::test_metadata_time_key);
    bp::def("test_metadata_contents_key", &metadata_key::test_metadata_contents_key);
    bp::def("test_metadata_choices_key", &metadata_key::test_metadata_choices_key);
    bp::def("test_metadata_repository_mask_info_key", &metadata_key::test_metadata_repository_mask_info_key);
    bp::def("test_metadata_keyword_name_set_key", &metadata_key::test_metadata_set_key<KeywordNameSet>);
    bp::def("test_metadata_string_set_key", &metadata_key::test_metadata_set_key<Set<std::string> >);
    bp::def("test_metadata_license_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<LicenseSpecTree>);
    bp::def("test_metadata_provide_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<ProvideSpecTree>);
    bp::def("test_metadata_dependency_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<DependencySpecTree>);
    bp::def("test_metadata_plain_text_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<PlainTextSpecTree>);
    bp::def("test_metadata_fetchable_uri_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<FetchableURISpecTree>);
    bp::def("test_metadata_simple_uri_spec_tree_key", &metadata_key::test_metadata_spec_tree_key<SimpleURISpecTree>);
    bp::def("test_metadata_section_key", &metadata_key::test_metadata_section_key);

    /**
     * Formatter tests
     */
    bp::def("test_plain_roles", &formatter::test_plain_roles);
    bp::def("test_acceptable_roles", &formatter::test_acceptable_roles);
    bp::def("test_package_roles", &formatter::test_package_roles);
    bp::def("test_can_space", &formatter::test_can_space);

    bp::def("test_formatter_string", &formatter::test_formatter_string);
    bp::def("test_formmater_keyword_name", &formatter::test_formmater_keyword_name);
    bp::def("test_formatter_license_spec_tree", &formatter::test_formatter_license_spec_tree);
    bp::def("test_formatter_provide_spec_tree", &formatter::test_formatter_provide_spec_tree);
    bp::def("test_formatter_dependency_spec_tree", &formatter::test_formatter_dependency_spec_tree);
    bp::def("test_formatter_plain_text_spec_tree", &formatter::test_formatter_plain_text_spec_tree);
    bp::def("test_formatter_simple_uri_spec_tree", &formatter::test_formatter_simple_uri_spec_tree);
    bp::def("test_formatter_fetchable_uri_spec_tree", &formatter::test_formatter_fetchable_uri_spec_tree);
}

