/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "packages_list_filtered_model.hh"
#include "packages_list_model.hh"
#include "packages_page.hh"
#include <algorithm>
#include <iterator>
#include <ctype.h>
#include <paludis/util/private_implementation_pattern-impl.hh>

using namespace gtkpaludis;
using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<PackagesListFilteredModel>
    {
        MainWindow * const main_window;
        PackagesPage * const packages_page;
        Glib::RefPtr<PackagesListModel> real_model;

        Implementation(MainWindow * const m, PackagesPage * const p,
                Glib::RefPtr<PackagesListModel> r) :
            main_window(m),
            packages_page(p),
            real_model(r)
        {
        }
    };
}

PackagesListFilteredModel::PackagesListFilteredModel(MainWindow * const m,
        PackagesPage * const p, Glib::RefPtr<PackagesListModel> d) :
    PrivateImplementationPattern<PackagesListFilteredModel>(new Implementation<PackagesListFilteredModel>(m, p, d)),
    Gtk::TreeModelFilter(d)
{
    set_visible_func(sigc::mem_fun(this, &PackagesListFilteredModel::handle_visible_func));
}

PackagesListFilteredModel::~PackagesListFilteredModel()
{
}

void
PackagesListFilteredModel::populate()
{
    refilter();
}

bool
PackagesListFilteredModel::handle_visible_func(const TreeModel::const_iterator & i) const
{
    bool result(true);

    if (result)
    {
        PackagesPackageFilterOption pfo(_imp->packages_page->get_package_filter());
        result &= ((*i)[_imp->real_model->columns().col_best_package_filter_option] >= pfo);
    }

    if (result)
    {
        std::string tft(_imp->packages_page->get_text_filter_text());
        if (! tft.empty())
        {
            std::string text;
            switch (_imp->packages_page->get_text_filter())
            {
                case ptfso_name:
                    text = Glib::ustring((*i)[_imp->real_model->columns().col_package]).raw();
                    break;

                case ptfso_description:
                    text = Glib::ustring((*i)[_imp->real_model->columns().col_description]).raw();
                    break;
            }

            if (! text.empty())
            {
                std::string text_lower, tft_lower;
                std::transform(text.begin(), text.end(), std::back_inserter(text_lower), &::tolower);
                std::transform(tft.begin(), tft.end(), std::back_inserter(tft_lower), &::tolower);
                result &= (std::string::npos != text_lower.find(tft_lower));
            }
        }
    }

    return result;
}

