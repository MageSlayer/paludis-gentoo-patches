## Unofficial Gentoo patches for Paludis package manager 

Paludis is a multi-format package manager.

Unfortunately, Paludis no longer officially supports Gentoo, thus this repo is a fork for keeping Gentoo support.

#### License
Same as original Paludis - GPLv2

#### Gentoo overlay
https://github.com/MageSlayer/paludis-gentoo-overlay

#### Official Exherbo site:
    http://paludis.exherbo.org/

### Incompatible Changes

  - Users of the scripting bindings will need to update their usage of
    `build_dependencies_key` to `build_dependencies_target_key` (the direct
    equivalent) and may want also to take `build_dependencies_host_key` into
    account.
