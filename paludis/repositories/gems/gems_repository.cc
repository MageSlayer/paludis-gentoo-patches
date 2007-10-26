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

#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/params.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/repositories/gems/gem_specification.hh>
#include <paludis/repositories/gems/gem_specifications.hh>
#include <paludis/repository_info.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/system.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/action.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <fstream>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<GemsRepository>
    {
        const gems::RepositoryParams params;

        const tr1::shared_ptr<Mutex> big_nasty_mutex;

        mutable tr1::shared_ptr<const CategoryNamePartSet> category_names;
        mutable MakeHashedMap<CategoryNamePart, tr1::shared_ptr<const QualifiedPackageNameSet> >::Type package_names;
        mutable MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type ids;

        mutable bool has_category_names;
        mutable bool has_ids;

        Implementation(const gems::RepositoryParams p, tr1::shared_ptr<Mutex> m = make_shared_ptr(new Mutex)) :
            params(p),
            big_nasty_mutex(m),
            has_category_names(false),
            has_ids(false)
        {
        }
    };
}

GemsRepository::GemsRepository(const gems::RepositoryParams & params) :
    Repository(RepositoryName("gems"),
            RepositoryCapabilities::create()
            .installed_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .mirrors_interface(0)
            .virtuals_interface(0)
            .provides_interface(0)
            .destination_interface(0)
            .e_interface(0)
            .qa_interface(0)
            .make_virtuals_interface(0)
            .hook_interface(0)
            .manifest_interface(0),
            "gems"),
    PrivateImplementationPattern<GemsRepository>(new Implementation<GemsRepository>(params))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("install_dir", stringify(_imp->params.install_dir));
    config_info->add_kv("builddir", stringify(_imp->params.builddir));
    config_info->add_kv("sync", _imp->params.sync);
    config_info->add_kv("sync_options", _imp->params.sync_options);

    _info->add_section(config_info);
}

GemsRepository::~GemsRepository()
{
}

void
GemsRepository::invalidate()
{
    Lock l(*_imp->big_nasty_mutex);

    _imp.reset(new Implementation<GemsRepository>(_imp->params, _imp->big_nasty_mutex));
}

void
GemsRepository::invalidate_masks()
{
    Lock l(*_imp->big_nasty_mutex);

    for (MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type::iterator it(_imp->ids.begin()), it_end(_imp->ids.end());
         it_end != it; ++it)
        for (PackageIDSequence::ConstIterator it2(it->second->begin()), it2_end(it->second->end());
             it2_end != it2; ++it2)
            (*it2)->invalidate_masks();
}

bool
GemsRepository::has_category_named(const CategoryNamePart & c) const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names->end() != _imp->category_names->find(c);
}

bool
GemsRepository::has_package_named(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_category_named(q.category))
        return false;

    need_ids();
    return _imp->package_names.find(q.category)->second->end() != _imp->package_names.find(q.category)->second->find(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
GemsRepository::category_names() const
{
    Lock l(*_imp->big_nasty_mutex);

    need_category_names();
    return _imp->category_names;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
GemsRepository::package_names(const CategoryNamePart & c) const
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
GemsRepository::package_ids(const QualifiedPackageName & q) const
{
    Lock l(*_imp->big_nasty_mutex);

    if (! has_package_named(q))
        return make_shared_ptr(new PackageIDSequence);

    need_ids();

    MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type::const_iterator i(
            _imp->ids.find(q));
    if (i == _imp->ids.end())
        return make_shared_ptr(new PackageIDSequence);

    return i->second;
}

void
GemsRepository::need_category_names() const
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
GemsRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    need_category_names();

    tr1::shared_ptr<QualifiedPackageNameSet> pkgs(new QualifiedPackageNameSet);
    _imp->package_names.insert(std::make_pair(CategoryNamePart("gems"), pkgs));

    Context context("When loading gems yaml file:");

    std::ifstream yaml_file(stringify(_imp->params.location / "yaml").c_str());
    if (! yaml_file)
        throw ConfigurationError("Gems yaml file '" + stringify(_imp->params.location / "yaml") + "' not readable");

    std::string output((std::istreambuf_iterator<char>(yaml_file)), std::istreambuf_iterator<char>());
    yaml::Document master_doc(output);
    gems::GemSpecifications specs(_imp->params.environment, shared_from_this(), *master_doc.top());

    for (gems::GemSpecifications::ConstIterator i(specs.begin()), i_end(specs.end()) ;
            i != i_end ; ++i)
    {
        pkgs->insert(i->first.first);

        MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<PackageIDSequence> >::Type::iterator v(_imp->ids.find(i->first.first));
        if (_imp->ids.end() == v)
            v = _imp->ids.insert(std::make_pair(i->first.first, make_shared_ptr(new PackageIDSequence))).first;

        v->second->push_back(i->second);
    }

    _imp->has_ids = true;
}

#if 0
void
GemsRepository::do_install(const tr1::shared_ptr<const PackageID> & id, const InstallOptions & o) const
{
    if (o.fetch_only)
        return;

    Command cmd(getenv_with_default("PALUDIS_GEMS_DIR", LIBEXECDIR "/paludis") +
            "/gems/gems.bash install '" + stringify(id->name().package) + "' '" + stringify(id->version()) + "'");
    cmd.with_stderr_prefix(stringify(*id) + "> ");
    cmd.with_setenv("GEMCACHE", stringify(_imp->params.location / "yaml"));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Install of '" + stringify(*id) + "' failed");
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
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }
    };
}

bool
GemsRepository::some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

