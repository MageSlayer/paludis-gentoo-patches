/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_INFO_METADATA_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/util/set.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class InfoPkgsMetadataKey :
            public MetadataSectionKey
        {
            private:
                Pimp<InfoPkgsMetadataKey> _imp;

            protected:
                void need_keys_added() const override;

            public:
                InfoPkgsMetadataKey(const Environment * const e,
                        const std::shared_ptr<const FSPathSequence> & f,
                        const ERepository * const);
                ~InfoPkgsMetadataKey() override;

                const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class InfoVarsMetadataKey :
            public MetadataCollectionKey<Set<std::string> >
        {
            private:
                Pimp<InfoVarsMetadataKey> _imp;

            public:
                InfoVarsMetadataKey(const std::shared_ptr<const FSPathSequence> &);
                ~InfoVarsMetadataKey() override;

                const std::shared_ptr<const Set<std::string> > parse_value() const override;

                const std::string raw_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                const std::string human_name() const override PALUDIS_ATTRIBUTE((warn_unused_result));
                MetadataKeyType type() const override PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const override PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }

    extern template class Pimp<erepository::InfoPkgsMetadataKey>;
    extern template class Pimp<erepository::InfoVarsMetadataKey>;
}

#endif
