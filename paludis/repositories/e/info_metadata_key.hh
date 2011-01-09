/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/set.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    struct ERepository;

    namespace erepository
    {
        class InfoPkgsMetadataKey :
            public MetadataSectionKey
        {
            private:
                Pimp<InfoPkgsMetadataKey> _imp;

            protected:
                virtual void need_keys_added() const;

            public:
                InfoPkgsMetadataKey(const Environment * const e,
                        const std::shared_ptr<const FSPathSequence> & f,
                        const ERepository * const);
                ~InfoPkgsMetadataKey();

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class InfoVarsMetadataKey :
            public MetadataCollectionKey<Set<std::string> >
        {
            private:
                Pimp<InfoVarsMetadataKey> _imp;

            public:
                InfoVarsMetadataKey(const std::shared_ptr<const FSPathSequence> &);
                ~InfoVarsMetadataKey();

                const std::shared_ptr<const Set<std::string> > value() const;

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<erepository::InfoPkgsMetadataKey>;
    extern template class Pimp<erepository::InfoVarsMetadataKey>;
}

#endif
