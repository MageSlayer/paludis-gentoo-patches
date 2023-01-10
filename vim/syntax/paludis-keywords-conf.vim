" Vim syntax file
" Language:     Paludis keywords.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis keywords.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisKeywordsConfComment start=/^\s*#/ end=/$/

syn match  PaludisKeywordsConfPDS /^[^ \t#\/]\+\/[^ \t#\/]\+\s*/
            \ nextgroup=PaludisKeywordsConfKeyword,PaludisKeywordsConfContinuation
            \ contains=PaludisKeywordsConfWildcard
syn match  PaludisKeywordsConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisKeywordsConfSet /^[^ \t#\/]\+\S\@!/
            \ nextgroup=PaludisKeywordsConfKeyword,PaludisKeywordsConfContinuation skipwhite
syn match  PaludisKeywordsConfKeyword contained /-\?\~\?[a-zA-Z0-9\-_*]\+/
            \ nextgroup=PaludisKeywordsConfKeyword,PaludisKeywordsConfContinuation skipwhite
syn match  PaludisKeywordsConfContinuation contained /\\$/
            \ nextgroup=PaludisKeywordsConfKeyword,PaludisKeywordsConfContinuation skipwhite skipnl

hi def link PaludisKeywordsConfComment          Comment
hi def link PaludisKeywordsConfPDS              Identifier
hi def link PaludisKeywordsConfWildcard         Special
hi def link PaludisKeywordsConfSet              Special
hi def link PaludisKeywordsConfKeyword          Keyword
hi def link PaludisKeywordsConfContinuation     Preproc

let b:current_syntax = "paludis-keywords-conf"

