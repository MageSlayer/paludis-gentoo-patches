/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_erasing_resume_file = make_format_string_fetcher("execute-resolution/erasing_resume_file", 1)
    << "\\nErasing resume file " << param<'f'>() << "..." << "\\n";

const auto fs_writing_resume_file = make_format_string_fetcher("execute-resolution/writing_resume_file", 1)
    << "\\nWriting resume information to " << param<'f'>() << "..." << "\\n";

const auto fs_starting_action = make_format_string_fetcher("execute-resolution/starting_action", 2)
    << "\\n" << c::bold_blue_or_pink() << param<'x'>() << ": Starting "
    << param<'a'>() << " for " << param<'i'>() << param<'r'>() << "..." << c::normal() << "\\n\\n";

const auto fs_done_action = make_format_string_fetcher("execute-resolution/done_action", 1)
    << c::bold_green_or_pink() << "Done " << param<'a'>() << " for " << param<'i'>()
    << param<'r'>() << c::normal() << "\\n\\n";

const auto fs_failed_action = make_format_string_fetcher("execute-resolution/failed_action", 1)
    << c::bold_red() << "Failed " << param<'a'>() << " for " << param<'i'>()
    << param<'r'>() << c::normal() << "\\n\\n";

const auto fs_special_set_world = make_format_string_fetcher("execute-resolution/special_set_world", 1)
    << "* Special set '" << param<'a'>() << "' does not belong in world" << "\\n";

const auto fs_not_removing_world = make_format_string_fetcher("execute-resolution/not_removing_world", 1)
    << "* Not removing '" << param<'a'>() << "'" << "\\n";

const auto fs_not_adding_world = make_format_string_fetcher("execute-resolution/not_adding_world", 1)
    << "* Not adding '" << param<'a'>() << "'" << "\\n";

const auto fs_updating_world = make_format_string_fetcher("execute-resolution/updating_world", 1)
    << "\\n" << c::bold_green_or_pink() << "Updating world" << c::normal() << "\\n\\n";

const auto fs_already_action = make_format_string_fetcher("execute-resolution/already_action", 1)
    << "\\n" << c::bold_green_or_pink() << param<'x'>() << ": Already "
    << param<'s'>() << " for " << param<'t'>() << c::normal() << "\\n\\n";

const auto fs_output_heading = make_format_string_fetcher("execute-resolution/output_heading", 1)
    << "\\n" << c::bold_yellow() << param<'h'>() << c::normal() << "\\n\\n";

const auto fs_no_output = make_format_string_fetcher("execute-resolution/no_output", 1)
    << "-> (no output for " << param<'n'>() << " seconds)" << "\\n";

const auto fs_summary_heading = make_format_string_fetcher("execute-resolution/summary_heading", 1)
    << "\\n" << c::bold_blue() << "Summary:" << c::normal() << "\\n\\n";

const auto fs_summary_job_pending = make_format_string_fetcher("execute-resolution/summary_job_pending", 1)
    << c::bold_yellow() << "pending:   " << c::normal() << param<'s'>() << "\\n";

const auto fs_summary_job_succeeded = make_format_string_fetcher("execute-resolution/summary_job_succeeded", 1)
    << c::bold_green_or_pink() << "succeeded: " << c::normal() << param<'s'>() << "\\n";

const auto fs_summary_job_failed = make_format_string_fetcher("execute-resolution/summary_job_failed", 1)
    << c::bold_red() << "failed:    " << c::normal() << param<'s'>() << "\\n";

const auto fs_summary_job_skipped = make_format_string_fetcher("execute-resolution/summary_job_skipped", 1)
    << c::bold_yellow() << "skipped:   " << c::normal() << param<'s'>() << "\\n";

