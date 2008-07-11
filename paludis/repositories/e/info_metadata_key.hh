/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/set.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace paludis
{
    namespace erepository
    {
        class InfoPkgsMetadataKey :
            public MetadataSectionKey,
            private PrivateImplementationPattern<InfoPkgsMetadataKey>
        {
            private:
                PrivateImplementationPattern<InfoPkgsMetadataKey>::ImpPtr & _imp;

            protected:
                virtual void need_keys_added() const;

            public:
                InfoPkgsMetadataKey(const Environment * const e,
                        const std::tr1::shared_ptr<const FSEntrySequence> & f, const std::string & p);
                ~InfoPkgsMetadataKey();
        };

        class InfoVarsMetadataKey :
            public MetadataCollectionKey<Set<std::string> >,
            private PrivateImplementationPattern<InfoVarsMetadataKey>
        {
            private:
                PrivateImplementationPattern<InfoVarsMetadataKey>::ImpPtr & _imp;

            public:
                InfoVarsMetadataKey(const std::tr1::shared_ptr<const FSEntrySequence> &);
                ~InfoVarsMetadataKey();

                const std::tr1::shared_ptr<const Set<std::string> > value() const;

               std::string pretty_print_flat(const Formatter<std::string> &) const;
        };
    }

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<erepository::InfoPkgsMetadataKey>;
    extern template class PrivateImplementationPattern<erepository::InfoVarsMetadataKey>;
#endif
}

#endif
