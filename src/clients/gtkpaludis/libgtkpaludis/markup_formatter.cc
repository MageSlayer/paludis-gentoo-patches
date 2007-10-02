/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "markup_formatter.hh"
#include "markup.hh"
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/set.hh>
#include <paludis/name.hh>

using namespace gtkpaludis;
using namespace paludis;

std::string
MarkupFormatter::format(const IUseFlag & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const IUseFlag & f, const format::Enabled &) const
{
    return markup_foreground("green", markup_escape(stringify(f.flag)));
}

std::string
MarkupFormatter::format(const IUseFlag & f, const format::Disabled &) const
{
    return markup_foreground("red", markup_escape("-" + stringify(f.flag)));
}

std::string
MarkupFormatter::format(const IUseFlag & f, const format::Forced &) const
{
    return markup_foreground("green", markup_escape("(" + stringify(f.flag) + ")"));
}

std::string
MarkupFormatter::format(const IUseFlag & f, const format::Masked &) const
{
    return markup_foreground("red", markup_escape("(-" + stringify(f.flag) + ")"));
}

std::string
MarkupFormatter::decorate(const IUseFlag &, const std::string & f, const format::Added &) const
{
    return f + "+";
}

std::string
MarkupFormatter::decorate(const IUseFlag &, const std::string & f, const format::Changed &) const
{
    return f + "*";
}

std::string
MarkupFormatter::format(const UseFlagName & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const UseFlagName & f, const format::Enabled &) const
{
    return markup_foreground("green", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const UseFlagName & f, const format::Disabled &) const
{
    return markup_foreground("red", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const UseFlagName & f, const format::Forced &) const
{
    return markup_foreground("green", markup_escape("(" + stringify(f) + ")"));
}

std::string
MarkupFormatter::format(const UseFlagName & f, const format::Masked &) const
{
    return markup_foreground("red", markup_escape("(" + stringify(f) + ")"));
}

std::string
MarkupFormatter::format(const UseDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const UseDepSpec & f, const format::Enabled &) const
{
    return markup_foreground("green", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const UseDepSpec & f, const format::Disabled &) const
{
    return markup_foreground("red", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const UseDepSpec & f, const format::Forced &) const
{
    return markup_foreground("green", markup_escape("(" + stringify(f) + ")"));
}

std::string
MarkupFormatter::format(const UseDepSpec & f, const format::Masked &) const
{
    return markup_foreground("red", markup_escape("(" + stringify(f) + ")"));
}

std::string
MarkupFormatter::format(const PackageDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const PackageDepSpec & f, const format::Installed &) const
{
    return markup_foreground("blue", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const PackageDepSpec & f, const format::Installable &) const
{
    return markup_foreground("darkblue", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const PlainTextDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const LicenseDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const LicenseDepSpec & f, const format::Accepted &) const
{
    return markup_foreground("green", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const LicenseDepSpec & f, const format::Unaccepted &) const
{
    return markup_foreground("red", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const KeywordName & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const KeywordName & f, const format::Accepted &) const
{
    return markup_foreground("green", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const KeywordName & f, const format::Unaccepted &) const
{
    return markup_foreground("red", markup_escape(stringify(f)));
}

std::string
MarkupFormatter::format(const std::string & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const LabelsDepSpec<URILabelVisitorTypes> & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const LabelsDepSpec<DependencyLabelVisitorTypes> & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const FetchableURIDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const SimpleURIDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const BlockDepSpec & f, const format::Plain &) const
{
    return markup_escape(stringify(f));
}

std::string
MarkupFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Plain &) const
{
    return markup_escape(stringify(*f));
}

std::string
MarkupFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Installed &) const
{
    return markup_foreground("blue", markup_escape(stringify(*f)));
}

std::string
MarkupFormatter::format(const tr1::shared_ptr<const PackageID> & f, const format::Installable &) const
{
    return markup_foreground("darkblue", markup_escape(stringify(*f)));
}

std::string
MarkupFormatter::newline() const
{
    return "\n";
}

std::string
MarkupFormatter::indent(const int i) const
{
    return std::string(4 * i, ' ');
}

