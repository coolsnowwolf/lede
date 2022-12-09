# ./packages folder

Add `.ipk` packages to this folder will allow the ImageBuilder to install them.

For more complex setups consider adding a custom feed containing packages.

    src custom file:///path/to/packages

Whenever the ImageBuilder builds a firmware image this folder will be reloaded
and a new package index created. In case signature checks are enabled the
`./packages/Packages` index will be signed with a locally generated key pair.
