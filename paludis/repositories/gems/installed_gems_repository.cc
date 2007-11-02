/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <paludis/repositories/gems/installed_gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repository_info.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/system.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/log.hh>
#include <paludis/util/strip.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/action.hh>

using namespace paludis;

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type IDMap;

namespace paludis
{
    template <>
    struct Implementation<InstalledGemsRepository>
    {
        const tr1::shared_ptr<Mutex> big_nasty_mutex;

        const gems::InstalledRepositoryParams params;

        mutable tr1::shared_ptr<const CategoryNamePartSet> category_names;
        mutable MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameSet> >::Type package_names;
        mutable IDMap ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        Implementation(const gems::InstalledRepositoryParams p,
                       tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
            big_nasty_mutex(m),
            params(p),
            has_category_names(false),
            has_ids(false)
        {
        }
    };
}

InstalledGemsRepository::InstalledGemsRepository(const gems::InstalledRepositoryParams & params) :
    Repository(RepositoryName("installed-gems"),
            RepositoryCapabilities::create()
            .installed_interface(this)
            .sets_interface(0)
            .syncable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .destination_interface(this)
            .e_interface(0)
            .qa_interface(0)
            .make_virtuals_interface(0)
            .hook_interface(0)
            .manifest_interface(0),
            "installed_gems"),
    PrivateImplementationPattern<InstalledGemsRepository>(new Implementation<InstalledGemsRepository>(params))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("install_dir", stringify(_imp->params.install_dir));
    config_info->add_kv("builddir", stringify(_imp->params.builddir));

    _info->add_section(config_info);
}

InstalledGemsRepository::~InstalledGemsRepository()
{
}

void
InstalledGemsRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);

    _imp.reset(new Implementation<InstalledGemsRepository>(_imp->params, _imp->big_nasty_mutex));
}

void
InstalledGemsRepository::invalidate_masks()
{
}

bool
InstalledGemsRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
InstalledGemsRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(q.category))
        return false;

    need_ids();
    return _imp->package_names.find(q.category)->second->end() != _imp->package_names.find(q.category)->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
InstalledGemsRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
InstalledGemsRepository::package_names(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(c))
        return make_shared_ptr(new QualifiedPackageNameSet);

    need_ids();

    MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameSet> >::Type::const_iterator i(
            _imp->package_names.find(c));
    if (i == _imp->package_names.end())
        return make_shared_ptr(new QualifiedPackageNameSet);
    return i->second;
}

tr1::shared_ptr<const PackageIDSequence>
InstalledGemsRepository::package_ids(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return make_shared_ptr(new PackageIDSequence);

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(q));
    if (i == _imp->ids.end())
        return make_shared_ptr(new PackageIDSequence);

    return i->second;
}

void
InstalledGemsRepository::need_category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_category_names)
        return;

    tr1::shared_ptr<CategoryNamePartSet> cat(new CategoryNamePartSet);
    _imp->category_names = cat;

    cat->insert(CategoryNamePart("gems"));
    _imp->has_category_names = true;
}

void
InstalledGemsRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    static CategoryNamePart gems("gems");

    Context c("When loading entries for repository '" + stringify(name()) + "':");

    need_category_names();

    tr1::shared_ptr<QualifiedPackageNameSet> pkgs(new QualifiedPackageNameSet);
    _imp->package_names.insert(std::make_pair(gems, pkgs));

    for (DirIterator d(_imp->params.install_dir / "specifications"), d_end ; d != d_end ; ++d)
    {
        if (! is_file_with_extension(*d, ".gemspec", IsFileWithOptions()))
            continue;

        std::string s(strip_trailing_string(d->basename(), ".gemspec"));
        std::string::size_type h(s.rfind('-'));
        if (std::string::npos == h)
        {
            Log::get_instance()->message(ll_qa, lc_context) << "Unrecognised file name format '"
                << *d << "' (no hyphen)";
            continue;
        }

        VersionSpec v(s.substr(h + 1));
        PackageNamePart p(s.substr(0, h));
        pkgs->insert(gems + p);

        if (_imp->ids.end() == _imp->ids.find(gems + p))
            _imp->ids.insert(std::make_pair(gems + p, make_shared_ptr(new PackageIDSequence)));
        _imp->ids.find(gems + p)->second->push_back(make_shared_ptr(new gems::GemSpecification(
                        _imp->params.environment, shared_from_this(), p, v, *d)));
    }
}

bool
InstalledGemsRepository::is_suitable_destination_for(const PackageID & e) const
{
    Lock l(*_imp->big_nasty_mutex);

    std::string f(e.repository()->format());
    return f == "gems";
}

bool
InstalledGemsRepository::is_default_destination() const
{
    return true;
}

bool
InstalledGemsRepository::want_pre_post_phases() const
{
    return true;
}

void
InstalledGemsRepository::merge(const MergeOptions &)
{
    throw InternalError(PALUDIS_HERE, "Invalid target for merge");
}

FSEntry
InstalledGemsRepository::root() const
{
    return FSEntry("/");
}

#if 0
void
InstalledGemsRepository::do_uninstall(const tr1::shared_ptr<const PackageID> & id,
        const UninstallOptions &) const
{
    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash uninstall '" + stringify(id->name().package) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEM_HOME", stringify(_imp->params.install_dir));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Uninstall of '" + stringify(*id) + "' failed");
}
#endif

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
            result = true;
        }
    };
}

bool
InstalledGemsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

