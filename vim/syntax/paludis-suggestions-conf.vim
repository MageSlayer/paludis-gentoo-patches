" Vim syntax file
" Language:	Paludis syggestions.conf files
" Author:	Ciaran McCreesh
" Copyright:	Copyright (c) 2007, 2010 Ciaran McCreesh
" Licence:	You may redistribute this under the same terms as Vim itself
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
	    \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfStars,PaludisSuggestionsConfContinuation
            \ contains=PaludisSuggestionsConfWildcard
syn match  PaludisSuggestionsConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisSuggestionsConfSet /^[^ \t#\/]\+\S\@!/
	    \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfStars,PaludisSuggestionsConfContinuation skipwhite
syn match  PaludisSuggestionsConfName contained /-\?[a-zA-Z0-9\-_]\+\/[a-zA-Z0-9\-_+]\+/
	    \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfStars,PaludisSuggestionsConfContinuation skipwhite
syn match  PaludisSuggestionsConfStars contained /-\?\*\/\*/
	    \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfStars,PaludisSuggestionsConfContinuation skipwhite
syn match  PaludisSuggestionsConfContinuation contained /\\$/
	    \ nextgroup=PaludisSuggestionsConfName,PaludisSuggestionsConfStars,PaludisSuggestionsConfContinuation skipwhite skipnl

hi def link PaludisSuggestionsConfComment          Comment
hi def link PaludisSuggestionsConfPDS              Identifier
hi def link PaludisSuggestionsConfWildcard         Special
hi def link PaludisSuggestionsConfSet              Special
hi def link PaludisSuggestionsConfName             Keyword
hi def link PaludisSuggestionsConfStars            Keyword
hi def link PaludisSuggestionsConfContinuation     Preproc

let b:current_syntax = "paludis-suggestions-conf"


