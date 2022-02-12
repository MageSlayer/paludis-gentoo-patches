## Unofficial Gentoo patches for Paludis package manager 

Paludis is a multi-format package manager.

Unfortunately, Paludis no longer officially supports Gentoo, thus this repo is a fork for keeping Gentoo support.

#### License
Same as original Paludis - GPLv2

#### Official Exherbo site:
    http://paludis.exherbo.org/

### Incompatible Changes

  - Users of the scripting bindings will need to update their usage of
    `build_dependencies_key` to `build_dependencies_target_key` (the direct
    equivalent) and may want also to take `build_dependencies_host_key` into
    account.
  - Likewise, `run_dependencies_key` was split into
    `run_dependencies_target_key` and `run_dependencies_host_key`.

### EAPI=8 Branch

**It is not completely safe to use this branch.**

The current implementation should be about feature-complete, with the exception
that IDEPEND dependencies are merged into RDEPEND and the
architecture-sensitive part is not implemented. This follows the EAPI=7 BDEPEND
implementation.

By now, it is pretty well tested.

However, the EAPI=7 changes are still incomplete and, since EAPI=8 builds on
the previous EAPI, it's not a good idea to use it yet.

It **may** break your system if you try to use it in the current state, mostly
due to the unimplemented empty OR/XOR group behavioral changes, which can lead
to incorrect dependencies being applied.

Until this branch is fully merged into the main repository, it **will** be
rebased without notice.

Packages known to fail:

| Package | Reason | State | Related to EAPI=8 | Has Workaround |
| ------- | ------ | ----- | ----------------- | -------------- |
| games-emulation/dolphin | variable in global scope | ❌ | ❌ | declare -g |
| dev-libs/mpfr | tries to apply all patches in `${DISTDIR}` -- portage creates a package-specific directory and copies all related distfiles to this directory, paludis just uses the global distdir | ❌ | ❌ | change `PATCHES+=( "${DISTDIR}"/ )` to `PATCHES+=( "${DISTDIR}/${MY_P}"*.patch )` |

**It is not completely safe to use this branch.**
