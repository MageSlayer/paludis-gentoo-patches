" Vim syntax file
" Language:	Paludis environment.conf files
" Author:	Ciaran McCreesh <ciaranm@gentoo.org>
" Copyright:	Copyright (c) 2007 Ciaran McCreesh
" Licence:	You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis environment.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
  finish
endif

syn region PaludisEnvironmentConfComment start=/^\s*#/ end=/$/

syn region PaludisEnvironmentConfKey start=/^\(\s*[^#]\)\@=/ end=/=\@=/
	    \ contains=PaludisEnvironmentConfKnownKey

syn match PaludisEnvironmentConfEquals /=/ skipwhite
	    \ nextgroup=PaludisEnvironmentConfValue

syn region PaludisEnvironmentConfValue contained start=// end=/$/
	    \ contains=PaludisEnvironmentConfString,PaludisEnvironmentConfUnquoted,
	    \    PaludisEnvironmentConfContinuation,PaludisEnvironmentConfVariable
	    \ skipwhite

syn match PaludisEnvironmentConfContinuation contained /\\$/
	    \ skipnl

syn match PaludisEnvironmentConfUnquoted contained /[^ \t$"'\\]\+/ skipwhite

syn region PaludisEnvironmentConfString contained start=/"/ end=/"/
	    \ contains=PaludisEnvironmentConfVariable
	    \ skipwhite

syn keyword PaludisEnvironmentConfKnownKey contained
	    \ reduced_username portage_compatible distribution

syn match PaludisEnvironmentConfVariable contained /\$\({[^}]\+}\|[a-zA-Z0-9_]\+\)/ skipwhite

hi def link PaludisEnvironmentConfKnownKey         Keyword
hi def link PaludisEnvironmentConfString           String
hi def link PaludisEnvironmentConfUnquoted         Constant
hi def link PaludisEnvironmentConfVariable         Identifier
hi def link PaludisEnvironmentConfContinuation     Preproc
hi def link PaludisEnvironmentConfComment          Comment

let b:current_syntax = "paludis-environment-conf"

