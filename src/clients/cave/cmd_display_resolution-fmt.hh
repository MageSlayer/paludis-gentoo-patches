/* vim: set sw=4 sts=4 et foldmethod=syntax : */

const auto fs_explanation_constraints_header  = make_format_string_fetcher("display-resolution/explanation_constraints_header", 1)
    << "    The following constraints were in action:" << "\\n";

const auto fs_explanation_constraint = make_format_string_fetcher("display-resolution/explanation_constraint", 1)
    << "      * " << param<'c'>() << "\\n";

const auto fs_explanation_constraint_reason  = make_format_string_fetcher("display-resolution/explanation_constraint_reason", 1)
    << "        Because of " << param<'r'>() << "\\n";

const auto fs_explanation_decision_heading  = make_format_string_fetcher("display-resolution/explanation_decision_heading", 1)
    << "    The decision made was:" << "\\n";

const auto fs_explanation_decision_untaken_heading  = make_format_string_fetcher("display-resolution/explanation_decision_untaken_heading", 1)
    << "    The decision made was not to:" << "\\n";

const auto fs_explanation_decision_existing_taken  = make_format_string_fetcher("display-resolution/explanation_decision_existing_taken", 1)
    << "        Use existing ID " << param<'i'>() << "\\n";

const auto fs_explanation_decision_existing_untaken  = make_format_string_fetcher("display-resolution/explanation_decision_existing_untaken", 1)
    << "        Do not take existing ID " << param<'i'>() << "\\n";

const auto fs_explanation_decision_remove_taken = make_format_string_fetcher("display-resolution/explanation_decision_remove_taken", 1)
    << "        Remove existing IDs" << "\\n";

const auto fs_explanation_decision_remove_untaken = make_format_string_fetcher("display-resolution/explanation_decision_remove_untaken", 1)
    << "        Do not remove existing IDs" << "\\n";

const auto fs_explanation_decision_remove_id = make_format_string_fetcher("display-resolution/explanation_decision_remove_id", 1)
    << "            Remove " << param<'i'>() << "\\n";

const auto fs_explanation_decision_nothing = make_format_string_fetcher("display-resolution/explanation_decision_nothing", 1)
    << "        Do not do anything" << "\\n";

const auto fs_explanation_decision_change_origin = make_format_string_fetcher("display-resolution/explanation_decision_change_origin", 1)
    << "        Use origin ID " << param<'i'>();

const auto fs_explanation_decision_change_via = make_format_string_fetcher("display-resolution/explanation_decision_change_via", 1)
    << " via binary created in " << param<'r'>();

const auto fs_explanation_decision_change_destination = make_format_string_fetcher("display-resolution/explanation_decision_change_destination", 1)
    << "\\n" << "        Install to repository " << param<'r'>() << "\\n";

const auto fs_explanation_decision_change_replacing = make_format_string_fetcher("display-resolution/explanation_decision_change_replacing", 1)
    << "            Replacing " << param<'i'>() << "\\n";

const auto fs_explanation_decision_unable_taken = make_format_string_fetcher("display-resolution/explanation_decision_unable_taken", 1)
    << "    No decision could be made" << "\\n";

const auto fs_explanation_decision_unable_untaken = make_format_string_fetcher("display-resolution/explanation_decision_unable_untaken", 1)
    << "    No decision could be made, but none was necessary" << "\\n";

const auto fs_explanation_decision_break_taken = make_format_string_fetcher("display-resolution/explanation_decision_break_taken", 1)
    << "    The decision made was to break " << param<'i'>() << "\\n";

const auto fs_explanation_decision_break_untaken = make_format_string_fetcher("display-resolution/explanation_decision_break_untaken", 1)
    << "    The decision made would be to break " << param<'i'>() << "\\n";

const auto fs_explaining = make_format_string_fetcher("display-resolution/explaining", 1)
    << "Explaining requested decisions:" << "\\n\\n";

const auto fs_explaining_resolvent = make_format_string_fetcher("display-resolution/explaining_resolvent", 1)
    << "For " << param<'r'>() << ":\\n";

const auto fs_description = make_format_string_fetcher("display-resolution/description", 1)
    << "    \"" << param<'s'>() << "\"" << "\\n";

const auto fs_choices_need_changes = make_format_string_fetcher("display-resolution/choices_need_changes", 1)
    << "    " << c::bold_red() << "Need changes for: " << c::normal() << param<'c'>()
    << param_if<'u'>() << " " << c::bold_normal() << "No changes needed: " << param<'u'>() << c::normal()
    << param_endif<'u'>() << "\\n";

const auto fs_choices = make_format_string_fetcher("display-resolution/choices", 1)
    << "    " << param<'u'>() << "\\n";

const auto fs_reasons_start = make_format_string_fetcher("display-resolution/reasons_start", 1)
    << "    ";

const auto fs_reasons = make_format_string_fetcher("display-resolution/reasons", 2)
    << "Reasons: ";

const auto fs_changes_reasons_start = make_format_string_fetcher("display-resolution/changes_reasons_start", 1)
    << c::bold_red() << "Reasons requiring changes: " << c::normal();

const auto fs_changes_reasons_end = make_format_string_fetcher("display-resolution/changes_reasons_end", 1)
    << " ";

const auto fs_reason_changes = make_format_string_fetcher("display-resolution/reason_changes", 1)
    << param<'c'>() << c::bold_yellow() << param<'r'>() << c::normal();

const auto fs_reason_special = make_format_string_fetcher("display-resolution/reason_special", 1)
    << param<'c'>() << c::bold_yellow() << param<'r'>() << c::normal();

const auto fs_reason_n_more = make_format_string_fetcher("display-resolution/reason_n_more", 1)
    << param<'c'>() << param<'n'>() << " more";

const auto fs_reason_normal = make_format_string_fetcher("display-resolution/reason_normal", 1)
    << param<'c'>() << c::yellow() << param<'r'>() << c::normal();

const auto fs_reasons_end = make_format_string_fetcher("display-resolution/reasons_end", 1)
    << "\\n";

const auto fs_confirm = make_format_string_fetcher("display-resolution/confirm", 1)
    << c::bold_red() << "    Cannot proceed without: " << c::normal() << param<'s'>() << "\\n";

const auto fs_take = make_format_string_fetcher("display-resolution/take", 2)
    << c::bold_green_or_pink() << "    Take using: " << c::normal() << "--take"
    << param_if<'g'>() << " (" << param<'g'>() << ")" << param_endif<'g'>()
    << "\\n";

const auto fs_take_purge = make_format_string_fetcher("display-resolution/take_purge", 1)
    << c::bold_green_or_pink() << "    Take using: " << c::normal() << "--purge" << "\\n";

const auto fs_mask_by = make_format_string_fetcher("display-resolution/mask_by", 1)
    << param<'i'>() << param<'k'>() << " " << param<'v'>() << "\\n";

const auto fs_mask_by_valueless = make_format_string_fetcher("display-resolution/mask_by_valueless", 1)
    << param<'i'>() << param<'k'>() << "\\n";

const auto fs_mask_by_repo_line = make_format_string_fetcher("display-resolution/mask_by_repo_line", 1)
    << param<'i'>() << param<'s'>() << "\\n";

const auto fs_masked_by = make_format_string_fetcher("display-resolution/masked_by", 2)
    << param<'i'>() << param<'c'>() << "Masked by " << c::normal() << param<'d'>()
    << param_if<'t'>() << c::blue_or_pink() << " [" << param<'t'>() << "]" << c::normal() << param_endif<'t'>()
    << "\\n";

const auto fs_masked_by_explanation = make_format_string_fetcher("display-resolution/masked_by_explanation", 1)
    << param<'i'>() << param<'c'>() << "Masked by " << c::normal() << param<'d'>() << " (" << param<'x'>() << ")" << "\\n";

const auto fs_download_megalots = make_format_string_fetcher("display-resolution/download_megalots", 1)
    << "    More than " << param<'i'>() << " to download" << "\\n";

const auto fs_download_amount = make_format_string_fetcher("display-resolution/download_amount", 1)
    << "    " << param<'i'>() << " to download" << "\\n";

const auto fs_change_type_new = make_format_string_fetcher("display-resolution/change_type_new", 1)
    << param<'c'>() << param<'s'>() << c::bold_blue();

const auto fs_change_type_slot_new = make_format_string_fetcher("display-resolution/change_type_slot_new", 1)
    << param<'c'>() << param<'s'>() << c::bold_blue();

const auto fs_change_type_add_to_slot = make_format_string_fetcher("display-resolution/change_type_add_to_slot", 1)
    << param<'c'>() << param<'s'>() << c::blue();

const auto fs_change_type_upgrade = make_format_string_fetcher("display-resolution/change_type_upgrade", 1)
    << param<'c'>() << param<'s'>() << c::blue();

const auto fs_change_type_reinstall = make_format_string_fetcher("display-resolution/change_type_reinstall", 1)
    << param<'c'>() << param<'s'>() << c::yellow();

const auto fs_change_type_downgrade = make_format_string_fetcher("display-resolution/change_type_downgrade", 1)
    << param<'c'>() << param<'s'>() << c::bold_yellow();

const auto fs_change_name = make_format_string_fetcher("display-resolution/change_name", 1)
    << param<'i'>() << c::normal();

const auto fs_change_not_best = make_format_string_fetcher("display-resolution/change_not_best", 1)
    << c::bold_yellow() << " (not the best version)" << c::normal();

const auto fs_change_formerly_from = make_format_string_fetcher("display-resolution/change_formerly_from", 1)
    << c::bold_yellow() << " (formerly from " << param<'r'>() << ")" << c::normal();

const auto fs_change_version = make_format_string_fetcher("display-resolution/change_version", 1)
    << " " << param<'v'>();

const auto fs_change_destination = make_format_string_fetcher("display-resolution/change_destination", 1)
    << " to ::" << param<'r'>();

const auto fs_change_via = make_format_string_fetcher("display-resolution/change_via", 1)
    << " via binary created in " << c::bold_normal() << param<'r'>() << c::normal();

const auto fs_change_replacing = make_format_string_fetcher("display-resolution/change_replacing", 1)
    << " replacing ";

const auto fs_change_replacing_one = make_format_string_fetcher("display-resolution/change_replacing_one", 1)
    << param<'c'>() << param<'s'>();

const auto fs_cycle_heading = make_format_string_fetcher("display-resolution/cycle_heading", 1)
    << " " << c::bold_normal() << "[cycle " << param<'s'>() << "]" << c::normal();

const auto fs_cycle_heading_error = make_format_string_fetcher("display-resolution/cycle_heading_error", 1)
    << " " << c::bold_red() << "[cycle " << param<'s'>() << "]" << c::normal();

const auto fs_cycle_notes = make_format_string_fetcher("display-resolution/cycle_notes", 1)
    << "    " << c::bold_normal() << param<'s'>() << c::normal() << "\\n";

const auto fs_cycle_notes_error = make_format_string_fetcher("display-resolution/cycle_notes_error", 1)
    << "    " << c::bold_red() << param<'s'>() << c::normal() << "\\n";

const auto fs_change_end = make_format_string_fetcher("display-resolution/change_end", 1)
    << "\\n";

const auto fs_uninstall_untaken = make_format_string_fetcher("display-resolution/uninstall_untaken", 1)
    << "(<) " << c::bold_green_or_pink() << param<'s'>() << c::normal() << " ";

const auto fs_uninstall_taken = make_format_string_fetcher("display-resolution/uninstall_taken", 1)
    << "<   " << c::bold_green_or_pink() << param<'s'>() << c::normal() << " ";

const auto fs_uninstall_version = make_format_string_fetcher("display-resolution/uninstall_version", 1)
    << param<'c'>() << param<'v'>();

const auto fs_uninstall_end = make_format_string_fetcher("display-resolution/uninstall_end", 1)
    << "\\n";

const auto fs_unable_untaken = make_format_string_fetcher("display-resolution/unable_untaken", 1)
    << "(!) " << c::red() << param<'s'>() << c::normal() << "\\n";

const auto fs_unable_taken = make_format_string_fetcher("display-resolution/unable_taken", 1)
    << "!   " << c::bold_red() << param<'s'>() << c::normal() << "\\n";

const auto fs_unable_unsuitable_header = make_format_string_fetcher("display-resolution/unable_unsuitable_header", 1)
    << "    Unsuitable candidates:" << "\\n";

const auto fs_unable_unsuitable_nothing = make_format_string_fetcher("display-resolution/unable_unsuitable_nothing", 1)
    << "      * Found no packages for resolvent " << param<'r'>() << "\\n";

const auto fs_unable_unsuitable_id = make_format_string_fetcher("display-resolution/unable_unsuitable_id", 1)
    << "      * " << param<'c'>() << param<'i'>() << c::normal() << "\\n";

const auto fs_unable_unsuitable_did_not_meet = make_format_string_fetcher("display-resolution/unable_unsuitable_did_not_meet", 1)
    << "        Did not meet " << param<'s'>() << "\\n";

const auto fs_unable_unsuitable_did_not_meet_additional = make_format_string_fetcher("display-resolution/unable_unsuitable_did_not_meet_additional", 1)
    << "            " << param<'s'>() << "\\n";

const auto fs_choice_to_explain_prefix = make_format_string_fetcher("display-resolution/choice_to_explain_prefix", 1)
    << param<'s'>() << ":" << "\\n";

const auto fs_choice_to_explain_all_same = make_format_string_fetcher("display-resolution/choice_to_explain_all_same", 1)
    << "    " << param<'s'>() << ":" << "%{column 34}" << " " << param<'d'>() << "\\n";

const auto fs_choice_to_explain_not_all_same = make_format_string_fetcher("display-resolution/choice_to_explain_not_all_same", 1)
    << "    " << param<'s'>() << ":" << "\\n";

const auto fs_choice_to_explain_one = make_format_string_fetcher("display-resolution/choice_to_explain_one", 1)
    << "        " << param<'s'>() << ":" << "%{column 34}" << " " << param<'d'>() << "\\n";

const auto fs_break_untaken = make_format_string_fetcher("display-resolution/break_untaken", 1)
    << "(X) " << c::bold_red() << param<'s'>() << c::normal() << " ";

const auto fs_break_taken = make_format_string_fetcher("display-resolution/break_taken", 1)
    << "X   " << c::bold_red() << param<'s'>() << c::normal() << " ";

const auto fs_break_id = make_format_string_fetcher("display-resolution/break_id", 1)
    << param<'i'>() << "\\n";

const auto fs_break_by = make_format_string_fetcher("display-resolution/break_by", 1)
    << "    Will be broken by uninstalls:\\n";

const auto fs_display_untaken = make_format_string_fetcher("display-resolution/display_untaken", 1)
    << "I did not take the following:\\n\\n";

const auto fs_display_unconfirmed = make_format_string_fetcher("display-resolution/display_uconfirmed", 1)
    << "I cannot proceed without being permitted to do the following:\\n\\n";

const auto fs_display_unorderable = make_format_string_fetcher("display-resolution/display_unorderable", 1)
    << "I cannot provide a legal ordering for the following:\\n\\n";

const auto fs_display_taken = make_format_string_fetcher("display-resolution/display_taken", 1)
    << "These are the actions I will take, in order:\\n\\n";

const auto fs_nothing_to_do = make_format_string_fetcher("display-resolution/nothing_to_do", 1)
    << "(nothing to do)\\n";

const auto fs_display_one_done = make_format_string_fetcher("display-resolution/display_one_done", 1)
    << "\\n";

const auto fs_display_done = make_format_string_fetcher("display-resolution/display_done", 2)
    << "";

const auto fs_totals_start = make_format_string_fetcher("display-resolution/totals_start", 1)
    << "Total: ";

const auto fs_totals_one = make_format_string_fetcher("display-resolution/totals_one", 1)
    << param<'c'>() << param<'n'>() << " " << param<'k'>();

const auto fs_totals_binaries = make_format_string_fetcher("display-resolution/totals_binaries", 1)
    << param<'c'>() << param<'n'>() << " binaries";

const auto fs_totals_uninstalls = make_format_string_fetcher("display-resolution/totals_uninstalls", 1)
    << param<'c'>() << param<'n'>() << " uninstalls";

const auto fs_totals_download_megalots = make_format_string_fetcher("display-resolution/totals_download_megalots", 1)
    << ", more than " << param<'n'>() << " to download";

const auto fs_totals_download_amount = make_format_string_fetcher("display-resolution/totals_download_amount", 1)
    << ", " << param<'n'>() << " to download";

const auto fs_totals_done = make_format_string_fetcher("display-resolution/totals_done", 1)
    << "\\n\\n";

const auto fs_display_errors = make_format_string_fetcher("display-resolution/display_errors", 1)
    << "I encountered the following errors:\\n\\n";

const auto fs_display_errors_untaken = make_format_string_fetcher("display-resolution/display_errors_untaken", 1)
    << "I encountered the following ignorable errors for untaken packages:\\n\\n";

