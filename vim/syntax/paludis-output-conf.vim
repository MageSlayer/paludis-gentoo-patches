" Vim syntax file
" Language:	Paludis output.conf files
" Author:	Ciaran McCreesh
" Copyright:	Copyright (c) 2010 Ciaran McCreesh
" Licence:	You may redistribute this under the same terms as Vim itself
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

syn match PaludisOutputConfEquals /=/ skipwhite
	    \ nextgroup=PaludisOutputConfValue

syn region PaludisOutputConfValue contained start=// end=/$/
	    \ contains=PaludisOutputConfString,PaludisOutputConfUnquoted,
	    \    PaludisOutputConfContinuation,PaludisOutputConfVariable,
	    \    PaludisOutputConfMacro,PaludisOutputConfKnownValue
	    \ skipwhite

syn match PaludisOutputConfContinuation contained /\\$/
	    \ skipnl

syn match PaludisOutputConfUnquoted contained /[^ \t$%"'\\]\+/ skipwhite

syn region PaludisOutputConfString contained start=/"/ end=/"/
	    \ contains=PaludisOutputConfVariable,PaludisOutputConfMacro
	    \ skipwhite

syn keyword PaludisOutputConfKnownKey contained
	    \ handler children messages_children type output_exclusivity
	    \ manager action ignore_unfetched if_success if_failure
	    \ child condition_variable if_true if_false if_unset
	    \ filename keep_on_empty keep_on_success summary_output_manager
	    \ format_debug format_info format_warn format_error format_log
	    \ summary_output_message start_command end_command
	    \ nothing_more_to_come_command succeeded_command
	    \ stdout_command stderr_command

syn keyword PaludisOutputConfKnownValue contained
	    \ buffer file format_messages forward_at_finish ipc tee standard command

syn match PaludisOutputConfVariable contained
            \ /\$\({[^}]\+}\|[a-zA-Z0-9_]\+\)/ skipwhite

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
hi def link PaludisOutputConfMacro                       Macro
hi def link PaludisOutputConfContinuation                Preproc
hi def link PaludisOutputConfComment                     Comment
hi def link PaludisOutputConfSection                     Type
hi def link PaludisOutputConfSectionName                 Special

let b:current_syntax = "paludis-repositories-conf"

