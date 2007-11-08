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
#include <paludis/dep_tree.hh>
#include <paludis/util/tr1_functional.hh>

namespace paludis
{
    namespace python
    {
        class PythonDepSpec;
        class PythonCompositeDepSpec;
        class PythonAllDepSpec;
        class PythonAnyDepSpec;
        class PythonUseDepSpec;
        class PythonStringDepSpec;
        class PythonPlainTextDepSpec;
        class PythonLicenseDepSpec;
        class PythonPackageDepSpec;
        class PythonFetchableURIDepSpec;
        class PythonSimpleURIDepSpec;
        class PythonBlockDepSpec;
        class PythonURILabelsDepSpec;
        class PythonDependencyLabelsDepSpec;
        class PythonNamedSetDepSpec;

        struct PythonDepSpecVisitorTypes :
            VisitorTypes<
                PythonDepSpecVisitorTypes,
                PythonDepSpec,
                PythonAnyDepSpec,
                PythonAllDepSpec,
                PythonUseDepSpec,
                PythonBlockDepSpec,
                PythonPlainTextDepSpec,
                PythonLicenseDepSpec,
                PythonPackageDepSpec,
                PythonFetchableURIDepSpec,
                PythonSimpleURIDepSpec,
                PythonURILabelsDepSpec,
                PythonDependencyLabelsDepSpec,
                PythonNamedSetDepSpec
            >
        {
        };

        class PALUDIS_VISIBLE PythonDepSpec :
            private InstantiationPolicy<PythonDepSpec, instantiation_method::NonCopyableTag>,
            public virtual ConstAcceptInterface<PythonDepSpecVisitorTypes>
        {
            protected:
                PythonDepSpec();

            public:
                virtual ~PythonDepSpec();

                virtual const PythonUseDepSpec * as_use_dep_spec() const
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

                void add_child(const tr1::shared_ptr<const PythonDepSpec>);

                typedef WrappedForwardIterator<enum ConstIteratorTag { },
                        const tr1::shared_ptr<const PythonDepSpec> > ConstIterator;

                ConstIterator begin() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        class PALUDIS_VISIBLE PythonAnyDepSpec :
            public PythonCompositeDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAnyDepSpec>
        {
            public:
                PythonAnyDepSpec();
                PythonAnyDepSpec(const AnyDepSpec &);
        };

        class PALUDIS_VISIBLE PythonAllDepSpec :
            public PythonCompositeDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonAllDepSpec>
        {
            public:
                PythonAllDepSpec();
                PythonAllDepSpec(const AllDepSpec &);
        };

        class PALUDIS_VISIBLE PythonUseDepSpec :
            public PythonCompositeDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonUseDepSpec>
        {
            private:
                const UseFlagName _flag;
                const bool _inverse;

            public:
                PythonUseDepSpec(const UseFlagName &, bool);
                PythonUseDepSpec(const UseDepSpec &);

                UseFlagName flag() const;
                bool inverse() const;
                virtual const PythonUseDepSpec * as_use_dep_spec() const;
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
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPackageDepSpec>
        {
            public:
                PythonPackageDepSpec(const PackageDepSpec &);
                PythonPackageDepSpec(const PythonPackageDepSpec &);
                ~PythonPackageDepSpec();

                static tr1::shared_ptr<const PythonPackageDepSpec>
                    make_from_string(const std::string &, const PackageDepSpecParseMode);

                operator PackageDepSpec() const;
                operator tr1::shared_ptr<PackageDepSpec>() const;

                tr1::shared_ptr<const QualifiedPackageName> package_ptr() const;
                tr1::shared_ptr<const PackageNamePart> package_name_part_ptr() const;
                tr1::shared_ptr<const CategoryNamePart> category_name_part_ptr() const;
                tr1::shared_ptr<const VersionRequirements> version_requirements_ptr() const;
                VersionRequirementsMode version_requirements_mode() const;
                void set_version_requirements_mode(const VersionRequirementsMode m);
                tr1::shared_ptr<const SlotName> slot_ptr() const;
                tr1::shared_ptr<const RepositoryName> repository_ptr() const;
                tr1::shared_ptr<const UseRequirements> use_requirements_ptr() const;
                tr1::shared_ptr<const DepTag> tag() const;
                void set_tag(const tr1::shared_ptr<const DepTag> & s);
                const tr1::shared_ptr<const PythonPackageDepSpec> without_use_requirements() const;

                std::string py_str() const;

                virtual const PythonPackageDepSpec * as_package_dep_spec() const;
        };

        class PALUDIS_VISIBLE PythonPlainTextDepSpec :
            public PythonStringDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonPlainTextDepSpec>
        {
            public:
                PythonPlainTextDepSpec(const std::string &);
                PythonPlainTextDepSpec(const PlainTextDepSpec &);
                PythonPlainTextDepSpec(const PythonPlainTextDepSpec &);
        };

        class PALUDIS_VISIBLE PythonNamedSetDepSpec :
            public PythonStringDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonNamedSetDepSpec>
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
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonSimpleURIDepSpec>
        {
            public:
                PythonSimpleURIDepSpec(const std::string &);
                PythonSimpleURIDepSpec(const SimpleURIDepSpec &);
        };

        class PALUDIS_VISIBLE PythonLicenseDepSpec :
            public PythonStringDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonLicenseDepSpec>
        {
            public:
                PythonLicenseDepSpec(const std::string &);
                PythonLicenseDepSpec(const LicenseDepSpec &);
        };

        class PALUDIS_VISIBLE PythonFetchableURIDepSpec :
            public PythonStringDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonFetchableURIDepSpec>
        {
            public:
                PythonFetchableURIDepSpec(const std::string &);
                PythonFetchableURIDepSpec(const FetchableURIDepSpec &);

                std::string original_url() const;
                std::string renamed_url_suffix() const;
        };

        class PALUDIS_VISIBLE PythonBlockDepSpec :
            public PythonStringDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonBlockDepSpec>
        {
            private:
                tr1::shared_ptr<const PythonPackageDepSpec> _spec;

            public:
                PythonBlockDepSpec(tr1::shared_ptr<const PythonPackageDepSpec> &);
                PythonBlockDepSpec(const BlockDepSpec &);

                tr1::shared_ptr<const PythonPackageDepSpec> blocked_spec() const;
        };

        class PALUDIS_VISIBLE PythonURILabelsDepSpec :
            public PythonDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonURILabelsDepSpec>
        {
            public:
                PythonURILabelsDepSpec(const std::string &);
                PythonURILabelsDepSpec(const URILabelsDepSpec &);
        };

        class PALUDIS_VISIBLE PythonDependencyLabelsDepSpec :
            public PythonDepSpec,
            public ConstAcceptInterfaceVisitsThis<PythonDepSpecVisitorTypes, PythonDependencyLabelsDepSpec>
        {
            public:
                PythonDependencyLabelsDepSpec(const std::string &);
                PythonDependencyLabelsDepSpec(const DependencyLabelsDepSpec &);
        };

        /**
         * Used to convert one of the SpecTrees to PythonDepSpec.
         */
        class SpecTreeToPython :
            public ConstVisitor<GenericSpecTree>,
            private InstantiationPolicy<SpecTreeToPython, instantiation_method::NonCopyableTag>
        {
            private:
                tr1::shared_ptr<PythonCompositeDepSpec> _current_parent;

            public:
                SpecTreeToPython();

                virtual ~SpecTreeToPython();

                const tr1::shared_ptr<const PythonDepSpec> result() const;

                void visit_sequence(const AllDepSpec &,
                        GenericSpecTree::ConstSequenceIterator,
                        GenericSpecTree::ConstSequenceIterator);

                void visit_sequence(const AnyDepSpec &,
                        GenericSpecTree::ConstSequenceIterator,
                        GenericSpecTree::ConstSequenceIterator);

                void visit_sequence(const UseDepSpec &,
                        GenericSpecTree::ConstSequenceIterator,
                        GenericSpecTree::ConstSequenceIterator);

                void visit_leaf(const PackageDepSpec &);

                void visit_leaf(const PlainTextDepSpec &);

                void visit_leaf(const BlockDepSpec &);

                void visit_leaf(const SimpleURIDepSpec &);

                void visit_leaf(const FetchableURIDepSpec &);

                void visit_leaf(const LicenseDepSpec &);

                void visit_leaf(const URILabelsDepSpec &);

                void visit_leaf(const DependencyLabelsDepSpec &);

                void visit_leaf(const NamedSetDepSpec &);
        };

        /**
         * Used to convert Python*DepSpec to one of the SpecTrees.
         */
        template <typename H_>
        class SpecTreeFromPython :
            public ConstVisitor<PythonDepSpecVisitorTypes>,
            private InstantiationPolicy<SpecTreeFromPython<H_>, instantiation_method::NonCopyableTag>
        {
            private:
                tr1::shared_ptr<ConstTreeSequence<H_, AllDepSpec> > _result;
                tr1::function<void (tr1::shared_ptr<ConstAcceptInterface<H_> >)> _add;

            public:
                SpecTreeFromPython();

                virtual ~SpecTreeFromPython();

                tr1::shared_ptr<typename H_::ConstItem> result() const;

                void visit(const PythonAllDepSpec &);
                void visit(const PythonAnyDepSpec &);
                void visit(const PythonUseDepSpec &);
                void visit(const PythonPackageDepSpec &);
                void visit(const PythonPlainTextDepSpec &);
                void visit(const PythonBlockDepSpec &);
                void visit(const PythonSimpleURIDepSpec &);
                void visit(const PythonFetchableURIDepSpec &);
                void visit(const PythonLicenseDepSpec &);
                void visit(const PythonURILabelsDepSpec &);
                void visit(const PythonDependencyLabelsDepSpec &);
                void visit(const PythonNamedSetDepSpec &);

                void real_visit(const PythonAllDepSpec &);
                void real_visit(const PythonAnyDepSpec &);
                void real_visit(const PythonUseDepSpec &);
                void real_visit(const PythonPackageDepSpec &);
                void real_visit(const PythonPlainTextDepSpec &);
                void real_visit(const PythonBlockDepSpec &);
                void real_visit(const PythonFetchableURIDepSpec &);
                void real_visit(const PythonSimpleURIDepSpec &);
                void real_visit(const PythonURILabelsDepSpec &);
                void real_visit(const PythonDependencyLabelsDepSpec &);
                void real_visit(const PythonLicenseDepSpec &);
                void real_visit(const PythonNamedSetDepSpec &);
        };
    }
}

#endif
