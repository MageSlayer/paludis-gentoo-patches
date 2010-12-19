" Vim syntax file
" Language:     Paludis licenses.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis licenses.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
  finish
endif

syn region PaludisLicensesConfComment start=/^\s*#/ end=/$/

syn match  PaludisLicensesConfPDS /^[^ \t#\/]\+\/[^ \t#\/]\+\s*/
            \ nextgroup=PaludisLicensesConfLicense,PaludisLicensesConfContinuation
            \ contains=PaludisLicensesConfWildcard
syn match  PaludisLicensesConfWildcard contained /\(\*\/\@=\|\/\@<=\*\)/
syn match  PaludisLicensesConfSet /^[^ \t#\/]\+\S\@!/
            \ nextgroup=PaludisLicensesConfLicense,PaludisLicensesConfContinuation skipwhite
syn match  PaludisLicensesConfLicense contained /-\?[a-zA-Z0-9\-_*]\+/
            \ nextgroup=PaludisLicensesConfLicense,PaludisLicensesConfContinuation skipwhite
syn match  PaludisLicensesConfContinuation contained /\\$/
            \ nextgroup=PaludisLicensesConfLicense,PaludisLicensesConfContinuation skipwhite skipnl

hi def link PaludisLicensesConfComment          Comment
hi def link PaludisLicensesConfPDS              Identifier
hi def link PaludisLicensesConfWildcard         Special
hi def link PaludisLicensesConfSet              Special
hi def link PaludisLicensesConfLicense          Keyword
hi def link PaludisLicensesConfContinuation     Preproc

let b:current_syntax = "paludis-licenses-conf"


