[[ -n "$MODULEPATH" ]] && export MODULEPATH="`eval echo $MODULEPATH`"
export EC_SHORTCUT_PATH=${SSMUSE_BASE:-${SSM_DOMAIN_BASE}}
s.use r.load.dot as s.ssmuse.dot
s.use r.load.dot as r.ssmuse.dot
[[ -n $BASH_VERSION ]] || { echo "WARNING: this is not a bash shell, auto completion not available" ; return ; }

#make_fix_the_paths

. init_shortcuts.dot
