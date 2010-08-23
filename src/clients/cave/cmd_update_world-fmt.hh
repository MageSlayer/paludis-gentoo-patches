/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_removed = make_format_string_fetcher("update-world/removed", 1)
    << "* Removed '" << param<'n'>() << "'" << "\\n";

const auto fs_did_not_remove = make_format_string_fetcher("update-world/did_not_remove", 1)
    << "* Nothing to remove for '" << param<'n'>() << "'" << "\\n";

const auto fs_added = make_format_string_fetcher("update-world/added", 1)
    << "* Added '" << param<'n'>() << "'" << "\\n";

const auto fs_did_not_add = make_format_string_fetcher("update-world/did-not-add", 1)
    << "* Did not need to add '" << param<'n'>() << "'" << "\\n";

