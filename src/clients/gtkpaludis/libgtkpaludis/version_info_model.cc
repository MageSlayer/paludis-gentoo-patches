/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "version_info_model.hh"
#include "query_window.hh"
#include "versions_page.hh"
#include "markup.hh"
#include "markup_formatter.hh"
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/util/strip.hh>
#include <paludis/package_id.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>
#include <paludis/mask.hh>
#include <paludis/metadata_key.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<VersionInfoModel>
    {
        QueryWindow * const query_window;
        VersionsPage * const versions_page;
        VersionInfoModel::Columns columns;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            query_window(m),
            versions_page(p)
        {
        }
    };
}

namespace
{
    struct PopulateDataItem
    {
        std::string key;
        std::string value_markup;

        PopulateDataItem(const std::string & k, const std::string & m) :
            key(k),
            value_markup(m)
        {
        }
    };
}

namespace gtkpaludis
{
    struct VersionInfoModel::PopulateData
    {
        std::list<PopulateDataItem> items;
    };
}


VersionInfoModel::VersionInfoModel(QueryWindow * const m, VersionsPage * const p) :
    PrivateImplementationPattern<VersionInfoModel>(new Implementation<VersionInfoModel>(m, p)),
    Gtk::TreeStore(_imp->columns)
{
}

VersionInfoModel::~VersionInfoModel()
{
}

VersionInfoModel::Columns::Columns()
{
    add(col_key);
    add(col_value_markup);
}

VersionInfoModel::Columns::~Columns()
{
}

VersionInfoModel::Columns &
VersionInfoModel::columns()
{
    return _imp->columns;
}

void
VersionInfoModel::populate()
{
    _imp->query_window->paludis_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionInfoModel::populate_in_paludis_thread),
                _imp->versions_page->get_id()), "Populating version information model");
}

namespace gtkpaludis
{
    struct VersionInfoModel::MetadataPopulator :
        ConstVisitor<MetadataKeyVisitorTypes>
    {
        MarkupFormatter formatter;

        virtual void got_key(const MetadataKey & k, const std::string & s) = 0;

        void visit(const MetadataSetKey<IUseFlagSet> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSetKey<Set<std::string> > & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSetKey<UseFlagNameSet> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSetKey<KeywordNameSet> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<DependencySpecTree> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<SimpleURISpecTree> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<FetchableURISpecTree> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<LicenseSpecTree> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<ProvideSpecTree> & k)
        {
             got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSpecTreeKey<RestrictSpecTree> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataSetKey<PackageIDSequence> & k)
        {
            got_key(k, k.pretty_print_flat(formatter));
        }

        void visit(const MetadataPackageIDKey & k)
        {
            got_key(k, markup_escape(stringify(*k.value())));
        }

        void visit(const MetadataStringKey & k)
        {
            got_key(k, markup_escape(stringify(k.value())));
        }

        void visit(const MetadataTimeKey & k)
        {
            char buf[255];
            time_t t(k.value());
            if (! strftime(buf, 254, "%c", gmtime(&t)))
                buf[0] = '\0';

            got_key(k, markup_escape(stringify(buf)));
        }

        void visit(const MetadataRepositoryMaskInfoKey & k)
        {
            if (! k.value())
                return;

            got_key(k, markup_escape(stringify(k.value()->mask_file) + ": " +
                        join(k.value()->comment->begin(), k.value()->comment->end(), " ")));
        }

        void visit(const MetadataFSEntryKey & k)
        {
            got_key(k, markup_escape(stringify(k.value())));
        }

        void visit(const MetadataContentsKey &)
        {
        }
    };

    struct VersionInfoModel::KeyMetadataPopulator :
        VersionInfoModel::MetadataPopulator
    {
        tr1::shared_ptr<VersionInfoModel::PopulateData> data;
        const MetadataKeyType type;

        KeyMetadataPopulator(const tr1::shared_ptr<VersionInfoModel::PopulateData> & d, const MetadataKeyType t) :
            data(d),
            type(t)
        {
        }

        virtual void got_key(const MetadataKey & k, const std::string & s)
        {
            if (k.type() != type)
                return;

            data->items.push_back(PopulateDataItem(k.human_name(), s));
        }
    };

    struct VersionInfoModel::MaskMetadataPopulator :
        VersionInfoModel::MetadataPopulator
    {
        std::string result;

        virtual void got_key(const MetadataKey &, const std::string & s)
        {
            result = s;
        }
    };

    struct VersionInfoModel::MaskPopulator :
        ConstVisitor<MaskVisitorTypes>
    {
        tr1::shared_ptr<VersionInfoModel::PopulateData> data;

        MaskPopulator(const tr1::shared_ptr<VersionInfoModel::PopulateData> & d) :
            data(d)
        {
        }

        void visit(const UserMask & k)
        {
            data->items.push_back(PopulateDataItem("Masked by " + strip_leading_string(k.description(), "by "),
                        markup_escape(k.description())));
        }

        void visit(const AssociationMask & k)
        {
            data->items.push_back(PopulateDataItem("Masked by " + strip_leading_string(k.description(), "by "),
                        markup_escape(stringify(*k.associated_package()))));
        }

        void visit(const UnsupportedMask & k)
        {
            data->items.push_back(PopulateDataItem("Masked by " + strip_leading_string(k.description(), "by "),
                        markup_escape(k.explanation())));
        }

        void visit(const RepositoryMask & k)
        {
            MaskMetadataPopulator p;
            if (k.mask_key())
                k.mask_key()->accept(p);
            data->items.push_back(PopulateDataItem("Masked by " + strip_leading_string(k.description(), "by "), p.result));
        }

        void visit(const UnacceptedMask & k)
        {
            MaskMetadataPopulator p;
            if (k.unaccepted_key())
                k.unaccepted_key()->accept(p);
            data->items.push_back(PopulateDataItem("Masked by " + strip_leading_string(k.description(), "by "), p.result));
        }
    };
}

void
VersionInfoModel::populate_in_paludis_thread(tr1::shared_ptr<const PackageID> p)
{
    tr1::shared_ptr<PopulateData> data(new PopulateData);
    if (p)
    {
        KeyMetadataPopulator mps(data, mkt_significant);
        std::for_each(indirect_iterator(p->begin_metadata()), indirect_iterator(p->end_metadata()), accept_visitor(mps));

        KeyMetadataPopulator mpn(data, mkt_normal);
        std::for_each(indirect_iterator(p->begin_metadata()), indirect_iterator(p->end_metadata()), accept_visitor(mpn));

        MaskPopulator mpm(data);
        std::for_each(indirect_iterator(p->begin_masks()), indirect_iterator(p->end_masks()), accept_visitor(mpm));
    }

    _imp->query_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionInfoModel::populate_in_gui_thread), data));
}

void
VersionInfoModel::populate_in_gui_thread(tr1::shared_ptr<const VersionInfoModel::PopulateData> names)
{
    clear();

    for (std::list<PopulateDataItem>::const_iterator i(names->items.begin()), i_end(names->items.end()) ;
            i != i_end ; ++i)
    {
        iterator r(append());
        (*r)[_imp->columns.col_key] = i->key;
        (*r)[_imp->columns.col_value_markup] = i->value_markup;
    }
}

