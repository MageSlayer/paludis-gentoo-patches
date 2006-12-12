#ifndef PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH
#define PALUDIS_GUARD_PALUDIS_CRAN_DEP_PARSER_HH 1

#include <paludis/dep_atom.hh>
#include <string>

namespace paludis
{
    /**
     * The CRANDepParser conversts a string representation of a CRAN Depends:
     * specification into a DepAtom instance.
     *
     * \ingroup grpdepparser
     */
    struct PALUDIS_VISIBLE CRANDepParser
    {
        /**
         * Parse function.
         */
        static DepAtom::ConstPointer parse(const std::string & s);
    };
}

#endif
