" Vim syntax file
" Language:     Paludis package_{un,}mask.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007, 2010 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis package_{un,}mask.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
  finish
endif

syn region PaludisPackageMaskConfComment start=/^\s*#/ end=/$/

syn match  PaludisPackageMaskConfPDS /^[^ \t#\/]\+\/[^ \t#\/]\+\s*/
            \ contains=PaludisPackageMaskConfWildcard
syn match  PaludisPackageMaskConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisPackageMaskConfSet /^[^ \t#\/]\+\S\@!/

hi def link PaludisPackageMaskConfComment          Comment
hi def link PaludisPackageMaskConfPDS              Identifier
hi def link PaludisPackageMaskConfSet              Special
hi def link PaludisPackageMaskConfWildcard         Special

let b:current_syntax = "paludis-package-mask-conf"



