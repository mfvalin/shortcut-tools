[[ -n "$MODULEPATH" ]] && export MODULEPATH="`eval echo $MODULEPATH`"
export EC_SHORTCUT_PATH=/ssm/net
[[ -n $BASH_VERSION ]] || { echo "ERROR: this is not a bash shell" ; return ; }

make_fix_the_paths
. init_shortcuts.dot
