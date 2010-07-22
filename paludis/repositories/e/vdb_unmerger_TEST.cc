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

#include <paludis/repositories/e/vdb_unmerger.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/package_database.hh>
#include <paludis/generator.hh>
#include <paludis/selection.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/filter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>

using namespace paludis;
using namespace test;

namespace
{
    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    bool ignore_nothing(const FSEntry &)
    {
        return false;
    }

    class VDBUnmergerNoDisplay :
        public VDBUnmerger
    {
        protected:

            void display(const std::string &) const
            {
            }

        public:

            VDBUnmergerNoDisplay(const VDBUnmergerOptions & o) :
                VDBUnmerger(o)
            {
            }
    };

    std::string fix(const std::string & s)
    {
        std::string result(s);
        std::replace(result.begin(), result.end(), ' ', '_');
        std::replace(result.begin(), result.end(), '\t', '_');
        return result;
    }

    class VDBUnmergerTest :
        public TestCase
    {
        public:
            const std::string what;
            FSEntry root_dir;
            std::string target;
            TestEnvironment env;
            std::shared_ptr<Repository> repo;
            std::shared_ptr<VDBUnmergerNoDisplay> unmerger;

            bool repeatable() const
            {
                return false;
            }

        protected:
            VDBUnmergerTest(const std::string & w) :
                TestCase("unmerge '" + w + "' test"),
                what(w),
                root_dir("vdb_unmerger_TEST_dir/root"),
                target(w)
            {
            }

            virtual void main_run() = 0;

        public:
            void run()
            {
                env.set_paludis_command("/bin/false");
                std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
                keys->insert("format", "vdb");
                keys->insert("names_cache", "/var/empty");
                keys->insert("provides_cache", "/var/empty");
                keys->insert("location", stringify(FSEntry::cwd() / "vdb_unmerger_TEST_dir" / "repo"));
                keys->insert("builddir", stringify(FSEntry::cwd() / "vdb_unmerger_TEST_dir" / "build"));
                repo = VDBRepository::repository_factory_create(&env, std::bind(from_keys, keys, std::placeholders::_1));
                env.package_database()->add_repository(0, repo);

                unmerger.reset(new VDBUnmergerNoDisplay(make_named_values<VDBUnmergerOptions>(
                                n::config_protect() = "/protected_file /protected_dir",
                                n::config_protect_mask() = "/protected_dir/unprotected_file /protected_dir/unprotected_dir",
                                n::environment() = &env,
                                n::ignore() = &ignore_nothing,
                                n::output_manager() = std::make_shared<StandardOutputManager>(),
                                n::package_id() = *env[selection::RequireExactlyOne(generator::Matches(
                                            parse_user_package_dep_spec("cat/" + fix(what), &env, UserPackageDepSpecOptions()),
                                            MatchPackageOptions()))]->begin(),
                                n::root() = root_dir
                                )));

                main_run();
            }
    };
}

namespace test_cases
{
    struct VDBUnmergerTestFileOk : VDBUnmergerTest
    {
        VDBUnmergerTestFileOk() : VDBUnmergerTest("file_ok") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_ok;

    struct VDBUnmergerTestFileWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestFileWithSpaces() : VDBUnmergerTest("file_ with spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_with_spaces;

    struct VDBUnmergerTestFileWithLotsOfSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestFileWithLotsOfSpaces() : VDBUnmergerTest("file_ with lots  of   spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_with_lots_of_spaces;

    struct VDBUnmergerTestFileWithTrailingSpace : VDBUnmergerTest
    {
        VDBUnmergerTestFileWithTrailingSpace() : VDBUnmergerTest("file_ with trailing  space\t ") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_with_trailing_space;

    struct VDBUnmergerTestFileBadType : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadType() : VDBUnmergerTest("file_bad_type") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_directory());
        }
    } test_vdb_unmerger_file_bad_type;

    struct VDBUnmergerTestFileBadMd5sum : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadMd5sum() : VDBUnmergerTest("file_bad_md5sum") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_file_bad_md5sum;

    struct VDBUnmergerTestFileBadMtime : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadMtime() : VDBUnmergerTest("file_bad_mtime") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_file_bad_mtime;

    struct VDBUnmergerTestFileReplacesDirectory : VDBUnmergerTest
    {
        VDBUnmergerTestFileReplacesDirectory() : VDBUnmergerTest("file_replaces_dir") { }

        void main_run()
        {
            unmerger->unmerge();
        }
    } test_vdb_unmerger_file_replaces_directory;

    struct VDBUnmergerTestDirOk : VDBUnmergerTest
    {
        VDBUnmergerTestDirOk() : VDBUnmergerTest("dir_ok") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_dir_ok;

    struct VDBUnmergerTestDirWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestDirWithSpaces() : VDBUnmergerTest("dir_ with spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_dir_with_spaces;

    struct VDBUnmergerTestDirWithLotsOfSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestDirWithLotsOfSpaces() : VDBUnmergerTest("dir_ with lots  of   spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_dir_with_lots_of_spaces;

    struct VDBUnmergerTestDirBadType : VDBUnmergerTest
    {
        VDBUnmergerTestDirBadType() : VDBUnmergerTest("dir_bad_type") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_dir_bad_type;

    struct VDBUnmergerTestDirNotEmpty : VDBUnmergerTest
    {
        VDBUnmergerTestDirNotEmpty() : VDBUnmergerTest("dir_not_empty") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_directory());
        }
    } test_vdb_unmerger_dir_not_empty;

    struct VDBUnmergerTestSymOk : VDBUnmergerTest
    {
        VDBUnmergerTestSymOk() : VDBUnmergerTest("sym_ok") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_ok;

    struct VDBUnmergerTestSymWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestSymWithSpaces() : VDBUnmergerTest("sym_ with spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_with_spaces;

    struct VDBUnmergerTestSymWithLotsOfSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestSymWithLotsOfSpaces() : VDBUnmergerTest("sym_ with lots  of   spaces") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_with_lots_of_spaces;

    struct VDBUnmergerTestSymWithManyArrows : VDBUnmergerTest
    {
        VDBUnmergerTestSymWithManyArrows() : VDBUnmergerTest("sym with many arrows") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_with_many_arrows;

    struct VDBUnmergerTestSymBadType : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadType() : VDBUnmergerTest("sym_bad_type") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_sym_bad_type;

    struct VDBUnmergerTestSymBadDst : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadDst() : VDBUnmergerTest("sym_bad_dst") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());
            TEST_CHECK(! (root_dir / "sym_dst_bad").exists());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_dst;

    struct VDBUnmergerTestSymBadMtime : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadMtime() : VDBUnmergerTest("sym_bad_mtime") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());

        }
    } test_vdb_unmerger_sym_bad_mtime;

    struct VDBUnmergerTestSymBadEntry1 : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadEntry1() : VDBUnmergerTest("sym_bad_entry_1") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_entry_1;

    struct VDBUnmergerTestSymBadEntry2 : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadEntry2() : VDBUnmergerTest("sym_bad_entry_2") { }

        void main_run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger->unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_entry_2;

    struct VDBUnmergerTestConfigProtect : VDBUnmergerTest
    {
        VDBUnmergerTestConfigProtect() : VDBUnmergerTest("config_protect") { }

        void main_run()
        {
            TEST_CHECK((root_dir / "protected_file").is_regular_file());
            TEST_CHECK((root_dir / "unprotected_file").is_regular_file());
            TEST_CHECK((root_dir / "protected_file_not_really").is_regular_file());

            TEST_CHECK((root_dir / "protected_dir/protected_file").is_regular_file());
            TEST_CHECK((root_dir / "protected_dir/unprotected_file").is_regular_file());
            TEST_CHECK((root_dir / "protected_dir/unprotected_file_not_really").is_regular_file());

            TEST_CHECK((root_dir / "protected_dir/unprotected_dir/unprotected_file").is_regular_file());

            TEST_CHECK((root_dir / "protected_dir/unprotected_dir_not_really/protected_file").is_regular_file());

            TEST_CHECK((root_dir / "protected_dir_not_really/unprotected_file").is_regular_file());

            unmerger->unmerge();

            TEST_CHECK((root_dir / "protected_file").exists());
            TEST_CHECK(! (root_dir / "unprotected_file").exists());
            TEST_CHECK(! (root_dir / "protected_file_not_really").exists());

            TEST_CHECK((root_dir / "protected_dir/protected_file").exists());
            TEST_CHECK(! (root_dir / "protected_dir/unprotected_file").exists());
            TEST_CHECK((root_dir / "protected_dir/unprotected_file_not_really").exists());

            TEST_CHECK(! (root_dir / "protected_dir/unprotected_dir/unprotected_file").exists());

            TEST_CHECK((root_dir / "protected_dir/unprotected_dir_not_really/protected_file").exists());

            TEST_CHECK(! (root_dir / "protected_dir_not_really/unprotected_file").exists());
        }
    } test_vdb_unmerger_config_protect;
}

