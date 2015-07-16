if tty -s 
then
#
_shortcut_compfn()
{
 local cur prev opts cmd
 cmd="${1##*/}"
 COMPREPLY=()
 cur="${COMP_WORDS[COMP_CWORD]}"
 COMPREPLY=( $(compgen -W "$(_complete_shortcut ${cur} )" ) )
 [[ $BASH_VERSION == 4* ]] && compopt -o nospace
 return 0
}
#
_echo_dir()
{
  _item=${1}
  [[ -d ${_item} ]] && echo ${_item}/
  [[ -f ${_item} ]] && echo ${_item}
}
#
_shortcut_possibilities()
{
for j in $(  r.shortcut-cfg --paths )
do
  if [[ -d $j ]] ; then
    cd $j
    for i in ${1}* ; do
     _echo_dir "$i"
     done
  fi
done
}
#
_complete_shortcut()
{
Liste="$(_shortcut_possibilities "${1:-[a-zA-Z0-9]}" | sed -e 's/[.]sh$//g' -e 's/[.]bndl$//' -e 's://:/:' | grep -v '[*]$' | sort -u)"
echo ${Liste:-${1:-[a-zA-Z0-9]}}
}
#
echo overriding alias shortcut
alias r.shortcut='. r.shortcut.dot'
complete -F _shortcut_compfn shortcut
#
#make_link_files_in_dir
#make_fix_the_paths
fi
[[ -n "$MODULEPATH" ]] && export MODULEPATH="`eval echo $MODULEPATH`"
export EC_SHORTCUT_PATH=/ssm/net
