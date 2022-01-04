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

**It is not safe to use this branch.**

The current implementation should be about feature-complete, with the exception
that IDEPEND dependencies are merged into RDEPEND and the
architecture-sensitive part is not implemented. This follows the EAPI=7 BDEPEND
implementation.

It is **not** well tested.

It **may** break your system if you try to use it in the current state.

More test cases are required.

While the current test suite executes successfully, most of the new features
are not hooked up into the testing system yet, so EAPI=8 ebuilds **may** show
undesired behavior.

**It is not safe to use this branch.**
