" Vim filetype detection file
" Language:	Paludis Things
" Author:	Ciaran McCreesh
" Copyright:	Copyright (c) 2007, 2010 Ciaran McCreesh
" Licence:	You may redistribute this under the same terms as Vim itself
"
" Filetype detection for Paludis things.
"

if &compatible || v:version < 700
    finish
endif

au BufNewFile,BufRead general.conf
    \     set filetype=paludis-general-conf

au BufNewFile,BufRead keywords.conf
    \     set filetype=paludis-keywords-conf

au BufNewFile,BufRead **/keywords.conf.d/*.conf
    \     set filetype=paludis-keywords-conf

au BufNewFile,BufRead platforms.conf
   \      set filetype=paludis-keywords-conf

au BufNewFile,BufRead **/platforms.conf.d/*.conf
   \      set filetype=paludis-keywords-conf

au BufNewFile,BufRead licenses.conf
    \     set filetype=paludis-licenses-conf

au BufNewFile,BufRead **/licenses.conf.d/*.conf
    \     set filetype=paludis-licenses-conf

au BufNewFile,BufRead licences.conf
   \      set filetype=paludis-licenses-conf

au BufNewFile,BufRead **/licences.conf.d/*.conf
   \      set filetype=paludis-licenses-conf

au BufNewFile,BufRead mirrors.conf
    \     set filetype=paludis-mirrors-conf

au BufNewFile,BufRead **/mirrors.conf.d/*.conf
    \     set filetype=paludis-mirrors-conf

au BufNewFile,BufRead package_mask.conf
    \     set filetype=paludis-package-mask-conf

au BufNewFile,BufRead **/package_mask.conf.d/*.conf
    \     set filetype=paludis-package-mask-conf

au BufNewFile,BufRead package_unmask.conf
    \     set filetype=paludis-package-mask-conf

au BufNewFile,BufRead **/package_unmask.conf.d/*.conf
    \     set filetype=paludis-package-mask-conf

au BufNewFile,BufRead use.conf
    \     set filetype=paludis-use-conf

au BufNewFile,BufRead **/use.conf.d/*.conf
    \     set filetype=paludis-use-conf

au BufNewFile,BufRead options.conf
   \      set filetype=paludis-use-conf

au BufNewFile,BufRead **/options.conf.d/*.conf
   \      set filetype=paludis-use-conf

au BufNewFile,BufRead repository_defaults.conf
    \     set filetype=paludis-repositories-conf

au BufNewFile,BufRead repository.template
    \     set filetype=paludis-repositories-conf

au BufNewFile,BufRead **/repositories/*.conf
    \     set filetype=paludis-repositories-conf

au BufNewFile,BufRead **/metadata/repository_mask.conf
    \     set filetype=paludis-package-mask-conf

