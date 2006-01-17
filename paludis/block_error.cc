/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "block_error.hh"

using namespace paludis;

BlockError::BlockError(const std::string & msg) throw () :
    DepListError("Block: " + msg)
{
}
