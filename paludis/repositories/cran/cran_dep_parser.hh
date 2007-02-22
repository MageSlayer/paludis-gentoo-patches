#ifndef PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH 1

#include <paludis/dep_spec.hh>
#include <string>

namespace paludis
{
    /**
     * The CRANDepParser conversts a string representation of a CRAN Depends:
     * specification into a DepSpec instance.
     *
     * \ingroup grpdepparser
     */
    struct PALUDIS_VISIBLE CRANDepParser
    {
        /**
         * Parse function.
         */
        static std::tr1::shared_ptr<const CompositeDepSpec> parse(const std::string & s);
    };
}

#endif
