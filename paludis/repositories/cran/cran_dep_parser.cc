#include <paludis/dep_spec.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_description.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/tokeniser.hh>

#include <string>
#include <list>

using namespace paludis;

tr1::shared_ptr<const CompositeDepSpec>
CRANDepParser::parse(const std::string & s, const PackageDepSpecParseMode mode)
{
    Context context("When parsing CRAN 'Depends:' string: '" + s + "':");

    tr1::shared_ptr<CompositeDepSpec> result(new AllDepSpec);
    Tokeniser<delim_kind::AnyOfTag, delim_mode::DelimiterTag> spec_tokeniser(",");

    std::list<std::string> specs;
    spec_tokeniser.tokenise(s, std::back_inserter(specs));
    std::list<std::string>::const_iterator a(specs.begin()), a_end(specs.end());
    for ( ; a != a_end ; ++a)
    {
        Context local_context("When processing token '" + *a + "':");

        std::string aa = strip_leading(strip_trailing(*a, ")"), " \t");

        std::string name, tmp, version, range;
        std::string::size_type p(aa.find('('));
        if ((std::string::npos != p))
        {
            name = strip_leading(strip_trailing(aa.substr(0, p), " \t"), " \t");
            tmp = aa.substr(p + 1);
            p = tmp.find(')');
            aa = tmp.substr(0, p);
            version = strip_trailing(strip_leading(aa, " \t(<>=~"), " )\t\n");
            range = strip_trailing(strip_leading(aa.substr(0, aa.find(version)), " \t"), " \t\n");
        }
        else
            name = strip_leading(strip_trailing(aa, " \t"), " \t");

        CRANDescription::normalise_name(name);
        CRANDescription::normalise_version(version);

        if ("R" == name)
            name = "dev-lang/R";
        else
            name = "cran/" + name;

        std::string spec_string;
        if (version.empty() || range.empty())
            spec_string = name;
        else
            spec_string = range + name + "-" + version;
        tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(spec_string, mode));
        result->add_child(spec);
    }

    return result;
}
