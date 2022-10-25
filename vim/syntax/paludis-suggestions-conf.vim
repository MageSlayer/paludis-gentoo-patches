" Vim syntax file
" Language:     Paludis syggestions.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007, 2010 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis syggestions.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisSuggestionsConfComment start=/^\s*#/ end=/$/

syn match  PaludisSuggestionsConfPDS /^[^ \t#\/]\+\/[^ \t#\/]\+\s*/
            \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfGroup,PaludisSuggestionsConfContinuation
            \ contains=PaludisSuggestionsConfWildcard
syn match  PaludisSuggestionsConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisSuggestionsConfGroup contained /-\?[a-zA-Z0-9\-_]\+\S\@!/
            \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfGroup,PaludisSuggestionsConfContinuation skipwhite
            \ contains=PaludisSuggestionsConfWildcard
syn match  PaludisSuggestionsConfName contained /-\?\(\*\|[a-zA-Z0-9\-_]\+\)\/\(\*\|[a-zA-Z0-9\-_+]\)\+/
            \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfGroup,PaludisSuggestionsConfContinuation skipwhite
            \ contains=PaludisSuggestionsConfWildcard
syn match  PaludisSuggestionsConfContinuation contained /\\$/
            \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfGroup,PaludisSuggestionsConfContinuation skipwhite skipnl

hi def link PaludisSuggestionsConfComment          Comment
hi def link PaludisSuggestionsConfPDS              Identifier
hi def link PaludisSuggestionsConfWildcard         Special
hi def link PaludisSuggestionsConfGroup            Macro
hi def link PaludisSuggestionsConfName             Keyword
hi def link PaludisSuggestionsConfContinuation     Preproc

let b:current_syntax = "paludis-suggestions-conf"

