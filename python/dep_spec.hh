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

#ifndef PALUDIS_GUARD_PYTHON_DEP_SPEC_HH
#define PALUDIS_GUARD_PYTHON_DEP_SPEC_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/spec_tree.hh>
#include <tr1/functional>

namespace paludis
{
    namespace python
    {
        class PythonDepSpec;
        class PythonCompositeDepSpec;
        class PythonAllDepSpec;
        class PythonAnyDepSpec;
        class PythonConditionalDepSpec;
        class PythonStringDepSpec;
        class PythonPlainTextDepSpec;
        class PythonLicenseDepSpec;
        class PythonPackageDepSpec;
        class PythonFetchableURIDepSpec;
        class PythonSimpleURIDepSpec;
        class PythonBlockDepSpec;
        class PythonURILabelsDepSpec;
        class PythonPlainTextLabelDepSpec;
        class PythonDependenciesLabelsDepSpec;
        class PythonNamedSetDepSpec;

        class PALUDIS_VISIBLE PythonDepSpec :
            private InstantiationPolicy<PythonDepSpec, instantiation_method::NonCopyableTag>,
            public virtual DeclareAbstractAcceptMethods<PythonDepSpec, MakeTypeList<
                    PythonAnyDepSpec,
                    PythonAllDepSpec,
                    PythonConditionalDepSpec,
                    PythonBlockDepSpec,
                    PythonPlainTextDepSpec,
                    PythonLicenseDepSpec,
                    PythonPackageDepSpec,
                    PythonFetchableURIDepSpec,
                    PythonSimpleURIDepSpec,
                    PythonURILabelsDepSpec,
                    PythonPlainTextLabelDepSpec,
                    PythonDependenciesLabelsDepSpec,
                    PythonNamedSetDepSpec
                >::Type>
        {
            protected:
                PythonDepSpec();

            public:
                virtual ~PythonDepSpec();

                virtual const PythonConditionalDepSpec * as_conditional_dep_spec() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const PythonPackageDepSpec * as_package_dep_spec() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE PythonCompositeDepSpec :
            public PythonDepSpec,
            private PrivateImplementationPattern<PythonCompositeDepSpec>
        {
            protected:
                PythonCompositeDepSpec();

            public:
                ~PythonCompositeDepSpec();

                void add_child(const std::tr1::shared_ptr<const PythonDepSpec>);

                struct ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<const PythonDepSpec> > ConstIterator;

                ConstIterator begin() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE PythonAnyDepSpec :
            public PythonCompositeDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonAnyDepSpec>
        {
            public:
                PythonAnyDepSpec();
                PythonAnyDepSpec(const AnyDepSpec &);
        };

        class PALUDIS_VISIBLE PythonAllDepSpec :
            public PythonCompositeDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonAllDepSpec>
        {
            public:
                PythonAllDepSpec();
                PythonAllDepSpec(const AllDepSpec &);
        };

        class PALUDIS_VISIBLE PythonConditionalDepSpec :
            public PythonCompositeDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonConditionalDepSpec>
        {
            private:
                const std::tr1::shared_ptr<const ConditionalDepSpecData> _data;

            public:
                PythonConditionalDepSpec(const ConditionalDepSpec &);

                virtual const PythonConditionalDepSpec * as_conditional_dep_spec() const;

                bool condition_met() const PALUDIS_ATTRIBUTE((warn_unused_result));
                bool condition_meetable() const PALUDIS_ATTRIBUTE((warn_unused_result));

                const std::tr1::shared_ptr<const ConditionalDepSpecData> data() const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE PythonStringDepSpec :
            public PythonDepSpec
        {
            private:
                std::string _str;

            protected:
                PythonStringDepSpec(const std::string &);
                PythonStringDepSpec(const StringDepSpec &);
                ~PythonStringDepSpec();

                void set_text(const std::string &);

            public:
                std::string text() const;
        };

        class PALUDIS_VISIBLE PythonPackageDepSpec :
            public PythonStringDepSpec,
            private PrivateImplementationPattern<PythonPackageDepSpec>,
            public ImplementAcceptMethods<PythonDepSpec, PythonPackageDepSpec>
        {
            public:
                PythonPackageDepSpec(const PackageDepSpec &);
                PythonPackageDepSpec(const PythonPackageDepSpec &);
                ~PythonPackageDepSpec();

                operator PackageDepSpec() const;
                operator std::tr1::shared_ptr<PackageDepSpec>() const;

                std::tr1::shared_ptr<const QualifiedPackageName> package_ptr() const;
                std::tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const;
                std::tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const;
                std::tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const;
                VersionRequirementsMode version_requirements_mode() const;
                void set_version_requirements_mode(const VersionRequirementsMode m);
                std::tr1::shared_ptr<const SlotRequirement> slot_requirement_ptr() const;
                std::tr1::shared_ptr<const RepositoryName> in_repository_ptr() const;
                std::tr1::shared_ptr<const RepositoryName> from_repository_ptr() const;
                std::tr1::shared_ptr<const AdditionalPackageDepSpecRequirements> additional_requirements_ptr() const;
                std::tr1::shared_ptr<const DepTag> tag() const;
                void set_tag(const std::tr1::shared_ptr<const DepTag> & s);
                const std::tr1::shared_ptr<const PythonPackageDepSpec> without_additional_requirements() const;

                std::string py_str() const;

                virtual const PythonPackageDepSpec * as_package_dep_spec() const;
        };

        class PALUDIS_VISIBLE PythonPlainTextDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonPlainTextDepSpec>
        {
            public:
                PythonPlainTextDepSpec(const std::string &);
                PythonPlainTextDepSpec(const PlainTextDepSpec &);
                PythonPlainTextDepSpec(const PythonPlainTextDepSpec &);
        };

        class PALUDIS_VISIBLE PythonNamedSetDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonNamedSetDepSpec>
        {
            private:
                const SetName _name;

            public:
                PythonNamedSetDepSpec(const SetName &);
                PythonNamedSetDepSpec(const NamedSetDepSpec &);

                const SetName name() const;
        };

        class PALUDIS_VISIBLE PythonSimpleURIDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonSimpleURIDepSpec>
        {
            public:
                PythonSimpleURIDepSpec(const std::string &);
                PythonSimpleURIDepSpec(const SimpleURIDepSpec &);
        };

        class PALUDIS_VISIBLE PythonLicenseDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonLicenseDepSpec>
        {
            public:
                PythonLicenseDepSpec(const std::string &);
                PythonLicenseDepSpec(const LicenseDepSpec &);
        };

        class PALUDIS_VISIBLE PythonFetchableURIDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonFetchableURIDepSpec>
        {
            public:
                PythonFetchableURIDepSpec(const std::string &);
                PythonFetchableURIDepSpec(const FetchableURIDepSpec &);

                std::string original_url() const;
                std::string renamed_url_suffix() const;
        };

        class PALUDIS_VISIBLE PythonBlockDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonBlockDepSpec>
        {
            private:
                std::tr1::shared_ptr<const PythonPackageDepSpec> _spec;
                bool _strong;

            public:
                PythonBlockDepSpec(const std::string &, const std::tr1::shared_ptr<const PythonPackageDepSpec> &, const bool);
                PythonBlockDepSpec(const BlockDepSpec &);

                std::tr1::shared_ptr<const PythonPackageDepSpec> blocking() const;
                bool strong() const;
        };

        class PALUDIS_VISIBLE PythonURILabelsDepSpec :
            public PythonDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonURILabelsDepSpec>
        {
            public:
                PythonURILabelsDepSpec(const std::string &);
                PythonURILabelsDepSpec(const URILabelsDepSpec &);
        };

        class PALUDIS_VISIBLE PythonPlainTextLabelDepSpec :
            public PythonStringDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonPlainTextLabelDepSpec>
        {
            public:
                PythonPlainTextLabelDepSpec(const std::string &);
                PythonPlainTextLabelDepSpec(const PlainTextLabelDepSpec &);
        };

        class PALUDIS_VISIBLE PythonDependenciesLabelsDepSpec :
            public PythonDepSpec,
            public ImplementAcceptMethods<PythonDepSpec, PythonDependenciesLabelsDepSpec>
        {
            public:
                PythonDependenciesLabelsDepSpec(const std::string &);
                PythonDependenciesLabelsDepSpec(const DependenciesLabelsDepSpec &);
        };

        /**
         * Used to convert one of the SpecTrees to PythonDepSpec.
         */
        class SpecTreeToPython :
            private InstantiationPolicy<SpecTreeToPython, instantiation_method::NonCopyableTag>
        {
            private:
                std::tr1::shared_ptr<PythonCompositeDepSpec> _current_parent;

            public:
                SpecTreeToPython();

                virtual ~SpecTreeToPython();

                const std::tr1::shared_ptr<const PythonDepSpec> result() const;

                void visit(const GenericSpecTree::NodeType<AllDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<AnyDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<ConditionalDepSpec>::Type &);

                void visit(const GenericSpecTree::NodeType<PackageDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<PlainTextDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<BlockDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<SimpleURIDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<FetchableURIDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<LicenseDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<PlainTextLabelDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<URILabelsDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<DependenciesLabelsDepSpec>::Type &);
                void visit(const GenericSpecTree::NodeType<NamedSetDepSpec>::Type &);
        };

        /**
         * Used to convert Python*DepSpec to one of the SpecTrees.
         */
        template <typename H_>
        class SpecTreeFromPython :
            private InstantiationPolicy<SpecTreeFromPython<H_>, instantiation_method::NonCopyableTag>
        {
            private:
                std::tr1::shared_ptr<H_> _result;
                std::tr1::shared_ptr<typename H_::BasicInnerNode> _add_to;

            public:
                SpecTreeFromPython();

                virtual ~SpecTreeFromPython();

                std::tr1::shared_ptr<H_> result() const;

                void visit(const PythonAllDepSpec &);
                void visit(const PythonAnyDepSpec &);
                void visit(const PythonConditionalDepSpec &);
                void visit(const PythonPackageDepSpec &);
                void visit(const PythonPlainTextDepSpec &);
                void visit(const PythonBlockDepSpec &);
                void visit(const PythonSimpleURIDepSpec &);
                void visit(const PythonFetchableURIDepSpec &);
                void visit(const PythonLicenseDepSpec &);
                void visit(const PythonURILabelsDepSpec &);
                void visit(const PythonPlainTextLabelDepSpec &);
                void visit(const PythonDependenciesLabelsDepSpec &);
                void visit(const PythonNamedSetDepSpec &);

                void real_visit(const PythonAllDepSpec &);
                void real_visit(const PythonAnyDepSpec &);
                void real_visit(const PythonConditionalDepSpec &);
                void real_visit(const PythonPackageDepSpec &);
                void real_visit(const PythonPlainTextDepSpec &);
                void real_visit(const PythonBlockDepSpec &);
                void real_visit(const PythonFetchableURIDepSpec &);
                void real_visit(const PythonSimpleURIDepSpec &);
                void real_visit(const PythonURILabelsDepSpec &);
                void real_visit(const PythonPlainTextLabelDepSpec &);
                void real_visit(const PythonDependenciesLabelsDepSpec &);
                void real_visit(const PythonLicenseDepSpec &);
                void real_visit(const PythonNamedSetDepSpec &);
        };
    }
}

#endif
