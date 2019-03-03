#!/bin/bash
# Copyright (c) 2019, Stein Gunnar Bakkeby, all rights reserved.
# Source this file in your .bashrc/.zshrc, then run the function to
# begin. Alternatively you can rename this function to something
# more to your own liking. If you do then remember to also update
# the references for the tab completion at the bottom of this file.

function gx () {
	if [ $ZSH_VERSION ]; then
		local func_name=$funcstack[1]
	else
		local func_name=${FUNCNAME[0]}
	fi

	if [[ ! -z "${_GXSETTINGS[filestore]}" ]]; then
		local filestore="${_GXSETTINGS[filestore]}"
	else
		local filestore=${HOME:-~}/.${func_name}.db
	fi

	typeset -A row
	local tmpifs=$IFS
	if [[ -e "$filestore" ]]; then
		while IFS="" read -r line || [ -n "$line" ]; do
			if [[ -z "$line" ]]; then
				continue
			fi
			row[${line%%=*}]=${line#*=}
		done < "$filestore"
	fi
	IFS=$'\n'

	if [ $ZSH_VERSION ]; then
		local keys=($(echo ${(k@)row} | tr ' ' '\n' | sort))
	else
		local keys=$(echo ${!row[@]} | tr ' ' '\n' | sort)
	fi

	if [[ $# == 0 ]]; then
		set -- -h
	fi

	local dirty=0
	local key=""
	local val=""
	local meta=""
	local param
	local i

	# Prioritise specific arguments so that they are processed before others
	for param in "--pin" "--tmp" "--temp"; do
		for i in $(seq 2 $#); do
			if [ $ZSH_VERSION ]; then
				val=${@[i]}
			else
				val="${!i}"
			fi
			if [[ "${val}" = "${param}" ]]; then
				set -- "${val}" "${@:1:$i-1}" "${@:$i+1:$#}"
			fi
		done
	done

	while (( $# )); do
		case "$1" in
		-a|--add)
			key=""
			val=""
			shift

			if [[ $# < 2 ]]; then
				echo "Error: Not enough arguments"
				echo "Usage: ${func_name} -a \"<command>\" <bookmark>"
				return
			fi

			if [[ "$1" =~ ^[^\ ./]+$ ]] && [[ ! "$2" =~ ^[^\ ./]+$ ]]; then
				val=$2
				key=$1
			else
				val=$1
				key=$2
			fi
			shift 2
			
			# No support for spaces and ¦ in bookmarks
			key=${key//[ ¦]/}
			
			if [[ -z "$key" ]]; then
				echo "Please specify a name to store this bookmark as"
			else
				local extra_text=""
				if [[ "$meta" =~ (¦|^)pin(¦|$) ]]; then
					extra_text="${extra_text} (pinned)"
				fi

				if [[ "$meta" =~ (¦|^)tmp(¦|$) ]]; then
					extra_text="${extra_text} (temp)"
				fi

				row[$key]="${val}${meta}"
				dirty=1
				echo "${val} added as ${key}${extra_text}"
				meta=""
			fi
			;;
		-r|--remove)
			shift
			if [[ $# > 0 ]]; then
				key=$1
				shift
			fi

			if [[ -z "$key" ]]; then
				key=${$(pwd)##*/}
			fi
			
			if [[ ! -z "${row[$key]}" ]]; then
				val=${row[$key]%%¦*}
				unset "row[$key]"
				echo "key '$key' referring to $val removed"
				dirty=1
			else
				echo "Bookmark with key \"$key\" does not exist"
			fi
			;;
		--clear)
			shift
			if [[ -e "$filestore" ]]; then
				rm -f -- "$filestore"
			fi
			;;
		--purge)
			shift
			for key in $keys; do
				if [[ "${row[$key]#*¦}" =~ (¦|^)pin(¦|$) ]]; then
					# pinned key, do not purge
					continue
				elif [[ "${row[$key]#*¦}" =~ (¦|^)tmp(¦|$) ]]; then
					# temporary key, explicitly purge
					echo "Removing temporary $key ➟ ${row[$key]%%¦*}"
					unset "row[$key]"
					dirty=1
					continue
				fi
			done
			;;
		--pin)
			shift
			if [[ $# > 0 ]] && [[ ! "$1" =~ ^- ]]; then
				key=$1
				shift
			fi

			if [[ -z "${key}" ]]; then
				meta="${meta}¦pin"
			elif [[ ! -z "${row[$key]}" ]]; then
				if [[ "${row[$key]#*¦}" =~ (¦|^)pin(¦|$) ]]; then
					echo "Bookmark with key \"$key\" is already pinned"
				else
					row[$key]="${row[$key]}¦pin"
					echo "Pinned key \"$key\""
					dirty=1
				fi
			else
				echo "Bookmark with key \"$key\" does not exist"
			fi
			;;
		--unpin)
			shift
			if [[ $# > 0 ]] && [[ ! "$1" =~ ^- ]]; then
				key=$1
				shift
			fi

			if [[ -z "${key}" ]]; then
				echo "Please specify the bookmark you wish to unpin"
			elif [[ ! -z "${row[$key]}" ]]; then
				if [[ "${row[$key]#*¦}" =~ (¦|^)pin(¦|$) ]]; then
					row[$key]="${row[$key]//¦pin/}"
					echo "Unpinned key \"$key\""
					dirty=1
				else
					echo "Bookmark with key \"$key\" is not pinned"
				fi
			else
				echo "Bookmark with key \"$key\" does not exist"
			fi
			;;
		--tmp|--temp)
			shift
			if [[ $# > 0 ]] && [[ ! "$1" =~ ^- ]]; then
				key=$1
				shift
			fi

			if [[ -z "${key}" ]]; then
				meta="${meta}¦tmp"
			elif [[ ! -z "${row[$key]}" ]]; then
				if [[ "${row[$key]#*¦}" =~ (¦|^)¦tmp(¦|$) ]]; then
					echo "Bookmark with key \"$key\" is already temporary"
				else
					row[$key]="${row[$key]}¦tmp"
					echo "Marked key \"$key\" as temporary"
					dirty=1
				fi
			else
				echo "Bookmark with key \"$key\" does not exist"
			fi
			;;
		--untmp|--untemp)
			shift
			if [[ $# > 0 ]] && [[ ! "$1" =~ ^- ]]; then
				key=$1
				shift
			fi

			if [[ -z "${key}" ]]; then
				echo "Please specify the bookmark you wish to unmark as being temporary"
			elif [[ ! -z "${row[$key]}" ]]; then
				if [[ "${row[$key]#*¦}" =~ (¦|^)tmp(¦|$) ]]; then
					row[$key]="${row[$key]//¦tmp/}"
					echo "Unmarked key \"$key\" as temporary"
					dirty=1
				else
					echo "Bookmark with key \"$key\" is not marked as being temporary"
				fi
			else
				echo "Bookmark with key \"$key\" does not exist"
			fi
			;;
		--description)
			shift
			if [[ -z "${key}" ]]; then
				meta="${meta}¦description=$1"
			else
				row[$key]="${row[$key]}¦description=$1"
			fi
			shift
			;;
		-l|--list)
			shift
			if [[ ${#row[@]} = 0 ]]; then
				echo "No bookmarks stored yet, try adding one with \"${func_name} -a . bookmark\""
				IFS=$tmpifs
				return
			fi

			local list_format="${_GXSETTINGS[list_format]:-%-17s%s%-10s\n}"
			local exists
			local command
			if [ $ZSH_VERSION ]; then
				local -a match
			fi
			for key in $keys; do
				exists=""
				if [[ "${row[$key]#*¦}" =~ (¦|^)pin(¦|$) ]]; then
					exists="${exists}${_GXSETTINGS[symbol_pin]:-🖈}"
				else
					if [[ "${row[$key]#*¦}" =~ (¦|^)tmp(¦|$) ]]; then
						exists="${exists}${_GXSETTINGS[symbol_tmp]:-ᴛ}"
					fi
				fi

				while [[ ${#exists} < 3 ]]; do
					exists="${exists} "
				done

				# Option to show a description rather than the actual code / command
				command="${row[$key]%%¦*}"
				if [[ "${row[$key]#*¦}" =~ (¦|^)description=([^¦]+)(¦|$) ]]; then
					if [ $ZSH_VERSION ]; then
						command="${match[2]}"
					else
						command="${BASH_REMATCH[2]}"
					fi
				fi
				printf "$list_format" "$key" "$exists" "$command"
			done
			echo
			;;
		--locate)
			shift
			echo "$filestore"
			;;
		--lookup)
			shift
			if [[ $# > 0 ]]; then
				echo "${row[$1]%%¦*}"
				shift
			fi
			;;
		--setup_aliases)
			shift
			for key in $keys; do
				alias "$key"="gx $key"
			done
			;;
		-k|--keys)
			shift
			echo "$keys"
			;;
		-o)
			shift
			# Internal option used for tab completion purposes
			echo "-a --add -l --list -r --remove --clear --pin --unpin --purge -h --help"\
				 "-k --keys --locate --temp --untemp --description --lookup --setup_aliases"
			;;
		-h|--help)
			shift
			local fmt="${_GXSETTINGS[help_format]:-  %-31s%s\n}"
			
			printf "%s\n\n" "Usage: ${func_name} [OPTION?] [BOOKMARK?]"
			printf "%s\n\n" "Bookmark often used commands and execute them later on the fly."
			printf "$fmt" "-a, --add" "adds a command with the given key"
			printf "$fmt" "-l, --list" "lists current bookmarks and commands"
			printf "$fmt" "-r, --remove" "removes a given bookmark"
			printf "$fmt" "    --clear" "removes all bookmarks"
			printf "$fmt" "    --purge" "removes temporary bookmarks, pinned bookmarks"
			printf "$fmt" "" "are not affected by purge"
			printf "$fmt" "    --pin" "pin a bookmark"
			printf "$fmt" "    --unpin" "removes the pin from a bookmark"
			printf "$fmt" "    --temp" "mark a bookmark as temporary"
			printf "$fmt" "    --untemp" "unmark a bookmark as temporary"
			printf "$fmt" "    --description" "adds a description to be shown when listing"
			printf "$fmt" "" "the bookmark rather than the actual command"
			printf "$fmt" "    --lookup" "show the command stored for a given bookmark"
			printf "$fmt" "-h, --help" "display this help section"
			printf "$fmt" "-k, --keys" "lists current keys"
			printf "$fmt" "    --locate" "list location of data file"
			printf "\n%s\n" "Examples:"
			printf "$fmt" "${func_name} -a \"ls -l ~\" lshome" "bookmarks the command as \"lshome\""
			printf "$fmt" "${func_name} -l" "lists currently stored commands"
			printf "$fmt" "${func_name} lshome" "executes the command stored as \"lshome\""
			printf "$fmt" "${func_name} -r lshome" "removes the bookmark with the key \"lshome\""
			printf "\n%s\n" "Warning! Do not use this product in any electronic equipment."

			if [ ! $ZSH_VERSION ]; then
				echo
			fi
			;;
		--) # end argument parsing
			shift
			break
			;;
		--*=) # unsupported flags
			echo "Error: Unsupported flag $1" >&2
			shift
			;;
		*) # positional arguments, look up key and execute command
			local key=$1
			local cmd=""
			local meta=""

			if [[ ! -z "${row[$key]}" ]]; then
				cmd=${row[$key]%%¦*}
				meta=${row[$key]#*¦}
			fi

			# Optionally other custom indexing solutions can be set
			# up as secondary lookups for unknown "keys" by adding
			# commands to to the _GXEXT array.
			if [[ -z "$cmd" ]]; then
				for ext_script in "${_GXEXT[@]}"; do
					if [[ ! -z "$ext_script" ]]; then
						cmd=$(eval $ext_script $@)
						if [ ! -z "$cmd" ]; then
							break
						fi
					fi
				done
			fi

			if [[ ! -z "$cmd" ]]; then
				shift
				if [[ "$cmd" != "" ]]; then
					if [[ "${_GXSETTINGS[print_cmd]:-0}" = "1" ]]; then
						echo "$cmd"
					fi
					if [[ "$cmd" =~ \\\$[0-9\{\@\#] ]]; then
						eval "$cmd"
					else
						eval "$cmd $@"
					fi
				fi
			elif [[ -d "$key" ]]; then
				builtin cd "$key" > /dev/null 2>&1
				eval "${_GXSETTINGS[post_cd_cmd]:-ls -a --color=auto}"
			else
				echo "Sorry, but '$key' is not a valid bookmark"
			fi

			break
			;;
		esac
	done
	
	if [[ $dirty == 1 ]]; then
		if [ $ZSH_VERSION ]; then
			local keys=($(echo ${(k@)row} | tr ' ' '\n' | sort))
		else
			local keys=$(echo ${!row[@]} | tr ' ' '\n' | sort)
		fi
		local output=""
		for key in $keys; do
			output="${output}${key}=${row[$key]}\n"
		done
		echo -e $output > "$filestore"
	fi
	
	IFS=$tmpifs
}

# Tab completion
if [ $ZSH_VERSION ]; then
	function _gxcomp {
		if [[ "${#words[*]}" == "2" ]]; then
			if [[ "${words[CURRENT]}" == '-'* ]]; then
				reply=($(gx -o))
			else
				reply=($(gx -k))
			fi
		else
			case ${words[2]} in
				-r|--remove|--pin|--unpin|--tmp|--untmp|--temp|--untemp|--lookup)
					reply=($(gx -k))
					;;
			esac
		fi
		return 0
	}
        
    compctl -K _gxcomp + -x \
	'c[-1,-l],c[-1,--list],c[-1,--clear],c[-1,--purge],c[-1,-h],c[-1,--help]' -k "()" - \
	'c[-1,-k],c[-1,--keys],c[-1,--locate],c[-1,--setup_aliases]' -k "()" - \
	's[prj/]' -s 'path/to/file' - \
	's[]' -/ -- \
	gx
else
	function _gxcomp {
		if [[ "${#COMP_WORDS[@]}" == "2" ]]; then
			if [[ "${COMP_WORDS[COMP_CWORD]}" == '-'* ]]; then
				COMPREPLY=($(compgen -W '$(gx -o)' -- ${COMP_WORDS[COMP_CWORD]}))
			else
				COMPREPLY=($(compgen -W '$(gx -k)' -- ${COMP_WORDS[COMP_CWORD]}))
			fi
		else
			case ${COMP_WORDS[1]} in
				-l|--list|--clear|--purge|-h|--help|-k|--keys|--locate|--lookup|--setup_aliases)
					COMPREPLY=($'\0'); # no further suggestions
					;;
				-r|--remove|--pin|--unpin|--tmp|--untmp|--temp|--untemp)
					COMPREPLY=($(compgen -W '$(gx -k)' -- ${COMP_WORDS[COMP_CWORD]}))
					;;
				-a|--add)
					COMPREPLY=($'\0'); # no further suggestions
					;;
				*)
					COMPREPLY=()
					;;
			esac
		fi
		return 0
	}
	shopt -s progcomp
	complete -o dirnames -F _gxcomp gx
fi