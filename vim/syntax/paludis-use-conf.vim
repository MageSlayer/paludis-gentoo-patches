" Vim syntax file
" Language:     Paludis use.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007, 2009, 2010 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis use.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisUseConfComment start=/^\s*#/ end=/$/

syn match  PaludisUseConfPDS /^[^ \t#\/]\+\/[^ \t#\/]\+\s*/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfPrefix,PaludisUseConfContinuation
            \ contains=PaludisUseConfWildcard
syn match  PaludisUseConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisUseConfSet /^[^ \t#\/]\+\S\@!/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfPrefix,PaludisUseConfContinuation skipwhite
syn match  PaludisUseConfFlag contained /\S\@<!(\?[a-zA-Z0-9\-\._*+]\+)\?\(\S\@!\|=\@=\)/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfFlagValue,PaludisUseConfPrefix,PaludisUseConfContinuation skipwhite
syn match  PaludisUseConfFlagValue contained /=\S*/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfPrefix,PaludisUseConfContinuation skipwhite
syn match  PaludisUseConfPrefix contained /[a-zA-Z0-9_*][a-zA-Z0-9\-_*]*:/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfPrefix,PaludisUseConfContinuation skipwhite
syn match  PaludisUseConfContinuation contained /\\$/
            \ nextgroup=PaludisUseConfFlag,PaludisUseConfPrefix,PaludisUseConfContinuation skipwhite skipnl

hi def link PaludisUseConfComment          Comment
hi def link PaludisUseConfPDS              Identifier
hi def link PaludisUseConfSet              Special
hi def link PaludisUseConfWildcard         Special
hi def link PaludisUseConfPrefix           Constant
hi def link PaludisUseConfFlag             Keyword
hi def link PaludisUseConfFlagValue        Type
hi def link PaludisUseConfContinuation     Preproc

let b:current_syntax = "paludis-use-conf"


