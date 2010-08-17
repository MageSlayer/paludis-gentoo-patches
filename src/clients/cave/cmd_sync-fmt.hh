/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_heading = make_format_string_fetcher("sync/heading", 1)
    << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "\\n\\n";

const auto fs_message_success = make_format_string_fetcher("sync/message_success", 1)
    << "* " << c::bold_green_or_pink() << param<'k'>() << ":" << c::normal() << "%{column 30}" << param<'v'>() << "\\n";

const auto fs_message_failure = make_format_string_fetcher("sync/message_failure", 1)
    << "* " << c::red() << param<'k'>() << ":" << c::normal() << "%{column 30}" << param<'v'>() << "\\n";

const auto fs_message_failure_message = make_format_string_fetcher("sync/message_failure_message", 1)
    << "    " << param<'s'>() << "\\n";

const auto fs_repos_title = make_format_string_fetcher("sync/repos_title", 1)
    << c::bold_normal() << "Repository%{column 30}Status%{column 52}Pending%{column 60}Active%{column 68}Done" << c::normal() << "\\n";

const auto fs_repo_starting = make_format_string_fetcher("sync/repo_starting", 1)
    << "-> " << c::bold_blue_or_pink() << param<'s'>() << c::normal() << "%{column 30}starting%{column 52}"
    << param<'p'>() << "%{column 60}" << param<'a'>() << "%{column 68}" << param<'d'>() << c::normal() << "\\n";

const auto fs_repo_done_success = make_format_string_fetcher("sync/repo_done_success", 1)
    << "-> " << c::bold_green_or_pink() << param<'s'>() << c::normal() << "%{column 30}success%{column 52}"
    << param<'p'>() << "%{column 60}" << param<'a'>() << "%{column 68}" << param<'d'>() << c::normal() << "\\n";

const auto fs_repo_done_no_syncing_required = make_format_string_fetcher("sync/repo_done_no_syncing_required", 1)
    << "-> " << c::bold_green_or_pink() << param<'s'>() << c::normal() << "%{column 30}no syncing required%{column 52}"
    << param<'p'>() << "%{column 60}" << param<'a'>() << "%{column 68}" << param<'d'>() << c::normal() << "\\n";

const auto fs_repo_done_failure = make_format_string_fetcher("sync/repo_done_failure", 1)
    << "-> " << c::bold_red() << param<'s'>() << c::normal() << "%{column 30}failed%{column 52}"
    << param<'p'>() << "%{column 60}" << param<'a'>() << "%{column 68}" << param<'d'>() << c::normal() << "\\n";

const auto fs_repo_active = make_format_string_fetcher("sync/repo_active", 1)
    << "-> " << c::bold_yellow() << param<'s'>() << c::normal() << "%{column 30}active%{column 52}"
    << param<'p'>() << "%{column 60}" << param<'a'>() << "%{column 68}" << param<'d'>() << c::normal() << "\\n";

const auto fs_repo_active_quiet = make_format_string_fetcher("sync/repo_active_quiet", 1)
    << "-> (no output for " << param<'s'>() << " seconds)\\n";

const auto fs_repo_tail = make_format_string_fetcher("sync/repo_tail", 1)
    << "    ... " << param<'s'>() << " seconds\\n";

