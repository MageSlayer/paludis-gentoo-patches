/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEMS_REPOSITORY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GEMS_GEMS_REPOSITORY_HH 1

#include <paludis/repository.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <string>

namespace paludis
{
    class PackageDatabase;

#include <paludis/repositories/gems/gems_repository-sr.hh>

    class PALUDIS_VISIBLE GemsRepository :
        public Repository,
        public RepositoryInstallableInterface,
        public RepositorySyncableInterface,
        public RepositorySetsInterface,
        private PrivateImplementationPattern<GemsRepository>
    {
        protected:
            virtual bool do_has_category_named(const CategoryNamePart &) const;

            virtual bool do_has_package_named(const QualifiedPackageName &) const;

            virtual std::tr1::shared_ptr<const CategoryNamePartCollection> do_category_names() const;

            virtual std::tr1::shared_ptr<const QualifiedPackageNameCollection> do_package_names(
                    const CategoryNamePart &) const;

            virtual std::tr1::shared_ptr<const VersionSpecCollection> do_version_specs(
                    const QualifiedPackageName &) const;

            virtual bool do_has_version(const QualifiedPackageName &, const VersionSpec &) const;

            virtual std::tr1::shared_ptr<const VersionMetadata> do_version_metadata(
                    const QualifiedPackageName &,
                    const VersionSpec &) const;

            virtual void do_install(const QualifiedPackageName &, const VersionSpec &,
                    const InstallOptions &) const;

            virtual std::tr1::shared_ptr<DepAtom> do_package_set(const SetName &) const;

            virtual std::tr1::shared_ptr<const SetsCollection> sets_list() const;

            virtual bool do_sync() const;

        public:
            virtual std::tr1::shared_ptr<const RepositoryInfo> info(bool verbose) const;

            GemsRepository(const GemsRepositoryParams &);
            ~GemsRepository();

            virtual void invalidate();
            virtual void regenerate_cache() const;
    };
}

#endif
