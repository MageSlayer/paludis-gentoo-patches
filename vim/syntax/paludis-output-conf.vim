" Vim syntax file
" Language:     Paludis output.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2010 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for output.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisOutputConfComment start=/^\s*#/ end=/$/

syn region PaludisOutputConfKey start=/^\(\s*[^#]\)\@=/ end=/=\@=/
            \ contains=PaludisOutputConfKnownKey

syn match PaludisOutputConfEquals /?\?=/ skipwhite
            \ nextgroup=PaludisOutputConfValue

syn region PaludisOutputConfValue contained start=// end=/$/
            \ contains=PaludisOutputConfString,PaludisOutputConfUnquoted,
            \    PaludisOutputConfContinuation,PaludisOutputConfVariable,
            \    PaludisOutputConfEnvVariable,
            \    PaludisOutputConfMacro,PaludisOutputConfKnownValue
            \ skipwhite

syn match PaludisOutputConfContinuation contained /\\$/
            \ skipnl

syn match PaludisOutputConfUnquoted contained /[^ \t$%"'\\]\+/ skipwhite

syn region PaludisOutputConfString contained start=/"/ end=/"/
            \ contains=PaludisOutputConfVariable,PaludisOutputConfMacro,
            \    PaludisOutputConfEnvVariable
            \ skipwhite

syn keyword PaludisOutputConfKnownKey contained
            \ action
            \ always_keep_output_logs
            \ child
            \ children
            \ condition_variable
            \ end_command
            \ extra_message_managers
            \ extra_output_managers
            \ filename
            \ format_debug
            \ format_error
            \ format_info
            \ format_log
            \ format_status
            \ format_warn
            \ handler
            \ if_failure
            \ if_false
            \ if_success
            \ if_true
            \ if_unset
            \ ignore_unfetched
            \ keep_on_empty
            \ keep_on_success
            \ log_path
            \ manager
            \ messages_children
            \ nothing_more_to_come_command
            \ output_exclusivity
            \ quiet
            \ start_command
            \ stderr_children
            \ stderr_command
            \ stdout_children
            \ stdout_command
            \ succeeded_command
            \ summary_output_manager
            \ summary_output_message
            \ type

syn keyword PaludisOutputConfKnownValue contained
            \ buffer
            \ command
            \ file
            \ format_messages
            \ forward_at_finish
            \ ipc
            \ standard
            \ tee

syn match PaludisOutputConfVariable contained
            \ /\$\({[^{}]\+}\|\(ENV{\)\@![a-zA-Z0-9_]\+\)/ skipwhite

syn match PaludisOutputConfEnvVariable contained
            \ /\$\({ENV{[^{}]\+}}\|ENV{[a-zA-Z0-9_]\+}\)/ skipwhite

syn match PaludisOutputConfMacro contained
            \ /%\({[^}]*}\|[a-zA-Z0-9_]\*\)/ skipwhite

syn region PaludisOutputConfSection start=/^\[/ end=/\]$/ skipwhite
            \ contains=PaludisOutputConfSectionName

syn keyword PaludisOutputConfSectionName contained
            \ rule manager

hi def link PaludisOutputConfKnownKey                    Keyword
hi def link PaludisOutputConfKnownValue                  Special
hi def link PaludisOutputConfString                      String
hi def link PaludisOutputConfUnquoted                    Constant
hi def link PaludisOutputConfVariable                    Identifier
hi def link PaludisOutputConfEnvVariable                 Statement
hi def link PaludisOutputConfMacro                       Macro
hi def link PaludisOutputConfContinuation                Preproc
hi def link PaludisOutputConfComment                     Comment
hi def link PaludisOutputConfSection                     Type
hi def link PaludisOutputConfSectionName                 Special

let b:current_syntax = "paludis-repositories-conf"

