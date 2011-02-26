/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_E_KEY_HH 1

#include <paludis/metadata_key.hh>
#include <paludis/util/set.hh>
#include <paludis/util/map-fwd.hh>
#include <paludis/repositories/e/eapi-fwd.hh>

namespace paludis
{
    class ERepository;

    namespace erepository
    {
        class ERepositoryID;

        class EDependenciesKey :
            public MetadataSpecTreeKey<DependencySpecTree>
        {
            private:
                Pimp<EDependenciesKey> _imp;

            public:
                EDependenciesKey(
                        const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::string &, const std::string &, const std::string &,
                        const std::shared_ptr<const DependenciesLabelSequence> &,
                        const MetadataKeyType);
                ~EDependenciesKey();

                virtual const std::shared_ptr<const DependencySpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const DependenciesLabelSequence> initial_labels() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EFetchableURIKey :
            public MetadataSpecTreeKey<FetchableURISpecTree>
        {
            private:
                Pimp<EFetchableURIKey> _imp;

            public:
                EFetchableURIKey(const Environment * const,
                        const std::shared_ptr<const ERepositoryID> &,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::string &,
                        const MetadataKeyType);
                ~EFetchableURIKey();

                virtual const std::shared_ptr<const FetchableURISpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const URILabel> initial_label() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ESimpleURIKey :
            public MetadataSpecTreeKey<SimpleURISpecTree>
        {
            private:
                Pimp<ESimpleURIKey> _imp;

            public:
                ESimpleURIKey(const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        const bool is_installed);
                ~ESimpleURIKey();

                virtual const std::shared_ptr<const SimpleURISpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EPlainTextSpecKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>
        {
            private:
                Pimp<EPlainTextSpecKey> _imp;

            public:
                EPlainTextSpecKey(const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        bool is_installed);
                ~EPlainTextSpecKey();

                virtual const std::shared_ptr<const PlainTextSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMyOptionsKey :
            public MetadataSpecTreeKey<PlainTextSpecTree>
        {
            private:
                Pimp<EMyOptionsKey> _imp;

            public:
                EMyOptionsKey(const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        bool);
                ~EMyOptionsKey();

                virtual const std::shared_ptr<const PlainTextSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ERequiredUseKey :
            public MetadataSpecTreeKey<RequiredUseSpecTree>
        {
            private:
                Pimp<ERequiredUseKey> _imp;

            public:
                ERequiredUseKey(const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        bool i);
                ~ERequiredUseKey();

                virtual const std::shared_ptr<const RequiredUseSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EProvideKey :
            public MetadataSpecTreeKey<ProvideSpecTree>
        {
            private:
                Pimp<EProvideKey> _imp;

            public:
                EProvideKey(const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        const bool is_installed);
                ~EProvideKey();

                virtual const std::shared_ptr<const ProvideSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class ELicenseKey :
            public MetadataSpecTreeKey<LicenseSpecTree>
        {
            private:
                Pimp<ELicenseKey> _imp;

            public:
                ELicenseKey(
                        const Environment * const,
                        const std::shared_ptr<const EAPIMetadataVariable> &,
                        const std::shared_ptr<const EAPI> &,
                        const std::string &, const MetadataKeyType,
                        bool is_installed);
                ~ELicenseKey();

                virtual const std::shared_ptr<const LicenseSpecTree> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string pretty_print_value(
                        const PrettyPrinter &,
                        const PrettyPrintOptions &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EContentsKey :
            public MetadataValueKey<std::shared_ptr<const Contents> >
        {
            private:
                Pimp<EContentsKey> _imp;

            public:
                EContentsKey(const std::string &, const std::string &, const FSPath &, const MetadataKeyType);
                ~EContentsKey();

                const std::shared_ptr<const Contents> value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class EMTimeKey :
            public MetadataTimeKey
        {
            private:
                Pimp<EMTimeKey> _imp;

            public:
                EMTimeKey(const std::string &, const std::string &, const FSPath &, const MetadataKeyType);
                ~EMTimeKey();

                Timestamp value() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };
    }
}

#endif
