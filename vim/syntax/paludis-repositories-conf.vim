" Vim syntax file
" Language:     Paludis repositories/*.conf files
" Author:       Ciaran McCreesh
" Copyright:    Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
" Licence:      You may redistribute this under the same terms as Vim itself
"
" Syntax highlighting for Paludis repositories/*.conf files.
"

if &compatible || v:version < 700
    finish
endif

if exists("b:current_syntax")
    finish
endif

syn region PaludisRepositoriesConfComment start=/^\s*#/ end=/$/

syn region PaludisRepositoriesConfKey start=/^\(\s*[^#]\)\@=/ end=/=\@=/
            \ contains=PaludisRepositoriesConfKnownKey,PaludisRepositoriesConfBadKey

syn match PaludisRepositoriesConfEquals /=/ skipwhite
            \ nextgroup=PaludisRepositoriesConfValue

syn region PaludisRepositoriesConfValue contained start=// end=/$/
            \ contains=PaludisRepositoriesConfString,PaludisRepositoriesConfUnquoted,
            \    PaludisRepositoriesConfContinuation,PaludisRepositoriesConfVariable,
            \    PaludisRepositoriesConfEnvVariable,
            \    PaludisRepositoriesConfBadTemplateVariable,PaludisRepositoriesConfMacro,
            \    PaludisRepositoriesConfBadMacro,PaludisRepositoriesConfKnownValue
            \ skipwhite

syn match PaludisRepositoriesConfContinuation contained /\\$/
            \ skipnl

syn match PaludisRepositoriesConfUnquoted contained /[^ \t$%"'\\]\+/ skipwhite

syn region PaludisRepositoriesConfString contained start=/"/ end=/"/
            \ contains=PaludisRepositoriesConfVariable,PaludisRepositoriesConfEnvVariable,
            \    PaludisRepositoriesConfBadTemplateVariable,
            \    PaludisRepositoriesConfMacro,PaludisRepositoriesConfBadMacro
            \ skipwhite

syn keyword PaludisRepositoriesConfKnownKey contained
            \ location distdir format builddir library sync root yaml_uri
            \ master_repository profiles pkgdir setsdir securitydir newsdir
            \ names_cache sync sync_options eclassdirs cache write_cache
            \ importance layout use_manifest manifest_hashes thin_manifests
            \ binary_uri_prefix binary_keywords_filter binary_destination binary_distdir
            \ eapi_when_unspecified eapi_when_unknown profile_eapi_when_unspecified
            \ name handler config_template config_filename

syn keyword PaludisRepositoriesConfBadKey contained
            \ world

syn keyword PaludisRepositoriesConfKnownValue contained
            \ use require ignore

syn match PaludisRepositoriesConfVariable contained
            \ /\$\({\(repository_template\)\@![^{}]\+}\|\(repository_template\|ENV{\)\@![a-zA-Z0-9_]\+\)/ skipwhite

syn match PaludisRepositoriesConfEnvVariable contained
            \ /\$\({ENV{[^{}]\+}}\|ENV{[a-zA-Z0-9_]\+}\)/ skipwhite

syn match PaludisRepositoriesConfBadTemplateVariable contained
            \ /\$\({repository_template[^}]*}\|repository_template[a-zA-Z0-9_]*\)/ skipwhite

syn match PaludisRepositoriesConfMacro contained
            \ /%\({repository_template[^}]*}\|repository_template[a-zA-Z0-9_]\*\)/ skipwhite

syn match PaludisRepositoriesConfBadMacro contained
            \ /%\({\(repository_template\)\@![^}]\+}\|\(repository_template\)\@![a-zA-Z0-9_]\+\)/ skipwhite

hi def link PaludisRepositoriesConfKnownKey                    Keyword
hi def link PaludisRepositoriesConfBadKey                      Error
hi def link PaludisRepositoriesConfKnownValue                  Keyword
hi def link PaludisRepositoriesConfString                      String
hi def link PaludisRepositoriesConfUnquoted                    Constant
hi def link PaludisRepositoriesConfVariable                    Identifier
hi def link PaludisRepositoriesConfEnvVariable                 Statement
hi def link PaludisRepositoriesConfBadTemplateVariable         Error
hi def link PaludisRepositoriesConfMacro                       Macro
hi def link PaludisRepositoriesConfBadMacro                    Error
hi def link PaludisRepositoriesConfContinuation                Preproc
hi def link PaludisRepositoriesConfComment                     Comment

let b:current_syntax = "paludis-repositories-conf"

