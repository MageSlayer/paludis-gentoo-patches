" Vim syntax file
" Language:	Paludis general.conf files
" Author:	Ciaran McCreesh
" Copyright:	Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
" Licence:	You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis general.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
  finish
endif

syn region PaludisGeneralConfComment start=/^\s*#/ end=/$/

syn region PaludisGeneralConfKey start=/^\(\s*[^#]\)\@=/ end=/=\@=/
	    \ contains=PaludisGeneralConfKnownKey

syn match PaludisGeneralConfEquals /=/ skipwhite
	    \ nextgroup=PaludisGeneralConfValue

syn region PaludisGeneralConfValue contained start=// end=/$/
	    \ contains=PaludisGeneralConfString,PaludisGeneralConfUnquoted,
	    \    PaludisGeneralConfContinuation,PaludisGeneralConfVariable
	    \ skipwhite

syn match PaludisGeneralConfContinuation contained /\\$/
	    \ skipnl

syn match PaludisGeneralConfUnquoted contained /[^ \t$"'\\]\+/ skipwhite

syn region PaludisGeneralConfString contained start=/"/ end=/"/
	    \ contains=PaludisGeneralConfVariable
	    \ skipwhite

syn keyword PaludisGeneralConfKnownKey contained
	    \ reduced_username portage_compatible distribution world

syn match PaludisGeneralConfVariable contained /\$\({[^}]\+}\|[a-zA-Z0-9_]\+\)/ skipwhite

hi def link PaludisGeneralConfKnownKey         Keyword
hi def link PaludisGeneralConfString           String
hi def link PaludisGeneralConfUnquoted         Constant
hi def link PaludisGeneralConfVariable         Identifier
hi def link PaludisGeneralConfContinuation     Preproc
hi def link PaludisGeneralConfComment          Comment

let b:current_syntax = "paludis-general-conf"

