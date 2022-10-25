" Vim syntax file
" Language:     Paludis mirrors.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007, 2010 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis mirrors.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisMirrorsConfComment start=/^\s*#/ end=/$/

syn match  PaludisMirrorsConfName /^[a-zA-Z0-9\-_+]\+\S\@!/
            \ nextgroup=PaludisMirrorsConfURI,PaludisMirrorsConfContinuation skipwhite
syn match  PaludisMirrorsConfSet /^[*]\S\@!/
            \ nextgroup=PaludisMirrorsConfURI,PaludisMirrorsConfContinuation skipwhite
syn match  PaludisMirrorsConfURI contained /[a-zA-Z0-9\-_+]\+:\S\+/
            \ nextgroup=PaludisMirrorsConfURI,PaludisMirrorsConfContinuation skipwhite
syn match  PaludisMirrorsConfContinuation contained /\\$/
            \ nextgroup=PaludisMirrorsConfURI,PaludisMirrorsConfContinuation skipwhite skipnl

hi def link PaludisMirrorsConfComment          Comment
hi def link PaludisMirrorsConfName             Identifier
hi def link PaludisMirrorsConfSet              Special
hi def link PaludisMirrorsConfURI              Keyword
hi def link PaludisMirrorsConfContinuation     Preproc

let b:current_syntax = "paludis-mirrors-conf"

