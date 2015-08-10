if tty -s 
then
  if [[ -n $BASH_VERSION ]]
  then
    #
    _r.shortcut_comp()
    {
    local cur prev opts cmd
    cmd="${1##*/}"
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    COMPREPLY=( $(compgen -W "$(_r.shortcut.dot ${cur} )" ) )  # explicit directory, shortcut or module
    [[ $BASH_VERSION == 4* ]] && compopt -o nospace
    return 0
    }
    #
#    _echo_dir()
#    {
#      _item=${1}
#      [[ -d ${_item} ]] && echo ${_item}/
#      [[ -f ${_item} ]] && echo ${_item}
#    }
    #
    _shortcut_possibilities()
    {
    for j in $(  r.shortcut-cfg --paths )
    do
      if [[ -d $j ]] ; then
        cd $j
        for Item in ${1}* ; do
          [[ -d ${Item} ]] && echo ${Item}/
          [[ -f ${Item} ]] && echo ${Item}
#        _echo_dir "$i"
        done
      fi
    done
    }
    #
    _s.ssmuse.dot()
    {
       _r.shortcut.dot "${1}"
    }
    #
    _r.shortcut.dot()
    {
    if [[ "${1}" == ./* || "${1}" == /* ]] ; then
      Liste="$(FindDottableDir ${1})"    # explicit path to directory
    else
      Liste="$(_shortcut_possibilities "${1:-[a-zA-Z0-9]}" | sed -e 's/[.]sh$//g' -e 's/[.]bndl$//' -e 's://:/:' | grep -v '[*]$' | sort -u)"
    fi
    echo ${Liste:-${1:-[a-zA-Z0-9]}}
    }
    #
    FindDottableName()
    {
    for i in ${PATH//:/\/${1:-NoTaRgEt}* } ; do [[ -r $i ]] && [[ -f $i ]] && [[ ! -x $i ]] && echo ${i##*/} ; done
    }
    #
    FindExecutableName()
    {
    for i in ${PATH//:/\/${1:-NoTaRgEt}* } ; do [[ -r $i ]] && [[ -f $i ]] && [[ -x $i ]] && echo ${i##*/} ; done
    }
    #
    FindDottableDir()
    {
    for i in ${1:-NoTaRgEt}* }
    do
      [[ -d $i ]] && [[ -r $i ]] && [[ -x $i ]] && echo ${i}/     # directory, readable, accessible
    done
    }
    #
    FindExecutableFile()
    {
    for i in ${1:-NoTaRgEt}* }
    do
      [[ -f $i ]] && [[ -r $i ]] && [[ -x $i ]] && echo ${i}      # file,  executable
      [[ -d $i ]] && [[ -r $i ]] && [[ -x $i ]] && echo ${i}/     # directory, readable, accessible
    done
    }
    #
    FindDottableFile()
    {
    for i in ${1:-NoTaRgEt}* }
    do
      [[ -f $i ]] && [[ -r $i ]] && [[ ! -x $i ]] && echo ${i}    # file, not executable
      [[ -d $i ]] && [[ -r $i ]] && [[ -x $i ]] && echo ${i}/     # directory, readable, accessible
    done
    }
    #
    _ExecutableCompletion()
    {
      local cur
      COMPREPLY=()
      cur=${COMP_WORDS[COMP_CWORD]}
      [[ $BASH_VERSION == 4* ]] && compopt -o nospace
      if [[ "${cur}" == ./* || "${cur}" == /* ]] ; then
        COMPREPLY=( $(compgen -W "$(FindExecutableFile ${cur} )" ) ) # file name, use file completion rules
      else
        COMPREPLY=( $(compgen -W "$(FindExecutableName ${cur} )" ) ) # completion for first token if a name
      fi
    }
    _DottableCompletion()
    {
      local cur
      COMPREPLY=()
      cur=${COMP_WORDS[COMP_CWORD]}
      [[ $BASH_VERSION == 4* ]] && compopt -o nospace
      if (( COMP_CWORD > 1 )) ; then           #  after first completed word
        Fname="_${COMP_WORDS[1]}"
        if [[ "$(type -t ${Fname} )" == function ]] ; then      # there is a function with the same name prefixed by _
          COMPREPLY=( $(compgen -W "$(${Fname} ${cur} )" ) )
        elif [[ -x "$TMPDIR/bin/${Fname}" ]] ; then             # there is a cached command with the same name prefixed by _
          COMPREPLY=( $(compgen -W "$(${Fname} ${cur} )" ) )
        elif [[ -n "$(which ${Fname} 2>/dev/null)" ]] ; then    # there is a command with the same name prefixed by _
          ln -sf "$(which ${Fname})" ${TMPDIR}/bin/${Fname}     # cache the command for next time
          COMPREPLY=( $(compgen -W "$(${Fname} ${cur} )" ) )
        else
          COMPREPLY=( $(compgen -f "${cur}" ) )                 # fallback, regular filename completion
        fi
      else
        if [[ "${cur}" == ./* || "${cur}" == /* ]] ; then
          COMPREPLY=( $(compgen -W "$(FindDottableFile ${cur} )" ) ) # file name, use file completion rules
        else
          COMPREPLY=( $(compgen -W "$(FindDottableName ${cur} )" ) ) # completion for first token if a name
        fi
      fi
    }
    #
    #
    echo overriding alias r.shortcut , adding auto-completion for . and source
    if [[ -d /unique/armnssm || -d /sb/software/areas ]] ; then
      alias shortcut='. r.shortcut.dot'
      complete -F _r.shortcut_comp shortcut
    fi
    alias r.shortcut='. r.shortcut.dot'
    complete -F _r.shortcut_comp r.shortcut
    complete -o nospace -F _DottableCompletion .
    complete -o nospace -F _DottableCompletion source
    complete -o nospace -F _ExecutableCompletion which
    #
    #make_link_files_in_dir
    #make_fix_the_paths
  fi
fi
[[ -n "$MODULEPATH" ]] && export MODULEPATH="`eval echo $MODULEPATH`"
export EC_SHORTCUT_PATH=/ssm/net
