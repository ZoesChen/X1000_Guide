#!/bin/bash

function gettop
{
	local TOPFILE=build/core/envsetup.mk
	if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
		echo $TOP
	else
		if [ -f $TOPFILE ] ; then
			# The following circumlocution (repeated below as well) ensures
			# that we record the true directory name and not one that is
			# faked up with symlink names.
			PWD= /bin/pwd
		else
			local HERE=$PWD
			T=
			while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
				\cd ..
				T=`PWD= /bin/pwd`
			done
			\cd $HERE
			if [ -f "$T/$TOPFILE" ]; then
				echo $T
			fi
		fi
	fi
}
unset LUNCH_MENU_CHOICES
function add_lunch_combo()
{
	local new_combo=$1
	local c
	for c in ${LUNCH_MENU_CHOICES[@]} ; do
		if [ "$new_combo" = "$c" ] ; then
			return
		fi
	done
	LUNCH_MENU_CHOICES=(${LUNCH_MENU_CHOICES[@]} $new_combo)
}
function print_lunch_menu()
{
	local uname=$(uname)
	echo
	echo "You're building on" $uname
	echo
	echo "Lunch menu... pick a combo:"

	local i=1
	local choice
	for choice in ${LUNCH_MENU_CHOICES[@]}
	do
		echo "     $i. $choice"
		i=$(($i+1))
	done

	echo
}

function get_build_var()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
		return
	fi
	CALLED_FROM_SETUP=true BUILD_SYSTEM=build/core \
	make --no-print-directory -C "$T" -f build/core/dumpvar.mk dumpvar-$1
}

function printconfig()
{
	T=$(gettop)
	if [ ! "$T" ]; then
		echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
		return
	fi
	get_build_var report_config
}

function findmakefile()
{
	TOPFILE=build/core/envsetup.mk
	local HERE=$PWD
	T=
	while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
		T=`PWD= /bin/pwd`
		if [ -f "$T/Build.mk" ]; then
			echo $T/Build.mk
			\cd $HERE
			return
		fi
		\cd ..
	done
	\cd $HERE
}


function mm()
{
	# If we're sitting in the root of the build tree, just do a
	# normal make.
	if [ -f build/core/envsetup.mk -a -f Makefile ]; then
		make $@
	else
		# Find the closest Android.mk file.
		T=$(gettop)
		local M=$(findmakefile)
		local MODULES=
		local GET_INSTALL_PATH=
		local ARGS=
		# Remove the path to top as the makefilepath needs to be relative
		local M=`echo $M|sed 's:'$T'/::'`
		if [ ! "$T" ]; then
			echo "Couldn't locate the top of the tree.  Try setting TOP."
		elif [ ! "$M" ]; then
			echo "Couldn't locate a makefile from the current directory."
		else
			for ARG in $@; do
				case $ARG in
					GET-INSTALL-PATH) GET_INSTALL_PATH=$ARG;;
				esac
			done
			if [ -n "$GET_INSTALL_PATH" ]; then
				MODULES=
				ARGS=GET-INSTALL-PATH
			else
				MODULES=all_modules
				ARGS=$@
			fi
			ONE_SHOT_MAKEFILE=$M make -C $T -f build/core/main.mk $MODULES $ARGS
		fi
	fi
}

function mma()
{
  if [ -f build/core/envsetup.mk -a -f Makefile ]; then
    make $@
  else
    T=$(gettop)
    if [ ! "$T" ]; then
      echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
    local MY_PWD=`PWD= /bin/pwd|sed 's:'$T'/::'`
    make -C $T -f build/core/main.mk $@ all_modules BUILD_MODULES_IN_PATHS="$MY_PWD"
  fi
}

function croot()
{
    T=$(gettop)
    if [ "$T" ]; then
        \cd $(gettop)
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function find_complite()
{
	T=$(gettop)
#
#	while true;do
#		if [ -d $T/prebuilts/toolchains ];then
#			echo compiler_path --> $T
#			break
#		fi
#
#		if [ x$T == x ];then
#			break
#		fi
#		T=${T%/*}
#	done
	TOOLCHAIN=$T/prebuilts/toolchains/mips-gcc472-glibc216/bin

	export DEVICE_COMPILER_PREFIX='mips-linux-gnu'
	export COMPILER_PATH=$T/prebuilts/toolchains/mips-gcc472-glibc216

	if [[ $PATH =~ $TOOLCHAIN ]];then
		echo "PATH=$PATH"
	else
		export PATH=$TOOLCHAIN:$PATH
	fi
}
function analysis_product()
{
	i=1
	TARGET_EXT_SUPPORT=
	while(true)
	do
		split=`echo $1|cut -d "_" -f$i`

		if [ "$split" != "" ];then
			if [ "$i" == "1" ];then
				export TARGET_DEVICE=$split
				echo $TARGET_DEVICE
			fi
			if [ "$i" == "2" ];then
				export TARGET_STORAGE_MEDIUM=$split
				echo $TARGET_STORAGE_MEDIUM
			fi
			if [ "$i" == "3" ];then
				export TARGET_EXT_SUPPORT=$split
				echo $TARGET_EXT_SUPPORT
			fi
			((i++))
		else

			break
		fi
	done

}

function patch_system()
{
	build/core/tools/gen_sys_script.py $TARGET_DEVICE $TARGET_STORAGE_MEDIUM $TARGET_EXT_SUPPORT
}

function autoenvsetup()
{
	build/core/tools/auto_env_setup.sh
}


function autotouchmk()
{
    T=$(gettop)
    $T/build/core/auto_touch_mk.sh
}

_complete_local_file()
{
	local file xspec i j
	local -a dirs files

	file=$1
	xspec=$2
	# Since we're probably doing file completion here, don't add a space after.
	if [[ $(type -t compopt) = "builtin" ]]; then
		compopt -o plusdirs
		if [[ "${xspec}" == "" ]]; then
			COMPREPLY=( ${COMPREPLY[@]:-} $(compgen -f -- "${cur}") )
		else
			compopt +o filenames
			COMPREPLY=( ${COMPREPLY[@]:-} $(compgen -f -X "${xspec}" -- "${cur}") )
		fi
	else
		# Work-around for shells with no compopt
		dirs=( $(compgen -d -- "${cur}" ) )
		if [[ "${xspec}" == "" ]]; then
			files=( ${COMPREPLY[@]:-} $(compgen -f -- "${cur}") )
		else
			files=( ${COMPREPLY[@]:-} $(compgen -f -X "${xspec}" -- "${cur}") )
		fi
		COMPREPLY=( $(
		for i in "${files[@]}"; do
			local skip=
			for j in "${dirs[@]}"; do
				if [[ $i == $j ]]; then
					skip=1
					break
				fi
			done
			[[ -n $skip ]] || printf "%s\n" "$i"
		done
		))
		COMPREPLY=( ${COMPREPLY[@]:-} $(
		for i in "${dirs[@]}"; do
			printf "%s/\n" "$i"
		done
		))
	fi
}

function _patch_system()
{
	local cur
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"

	_complete_local_file "${cur}"

	return 0
}
complete -F _patch_system patch_system

function lunch()
{
	local answer

	if [ "$1" ] ; then
		answer=$1
	else
		print_lunch_menu
		echo -n "Which would you like? [phoenix_nor_userdebug] "
		read answer
	fi

	local selection=

	if [ -z "$answer" ]
	then
		selection=phoenix_nor_userdebug
	elif (echo -n $answer | grep -q -e "^[0-9][0-9]*$")
	then
		if [ $answer -le ${#LUNCH_MENU_CHOICES[@]} ]
		then
			selection=${LUNCH_MENU_CHOICES[$(($answer-1))]}
		fi
	elif (echo -n $answer | grep -q -e "^[^\-][^\-]*-[^\-][^\-]*$")
	then
		selection=$answer
	fi

	if [ -z "$selection" ]
	then
		echo
		echo "Invalid lunch combo: $answer"
		return 1
	fi
	export TARGET_BUILD_APPS=

	local product=$(echo -n $selection | sed -e "s/-.*$//")
	echo selection is $selection
	echo product is $product
#	check_product $product
	if [ $? -ne 0 ]
	then
		echo
		echo "** Don't have a product spec for: '$product'"
		echo "** Do you have the right repo manifest?"
		product=
	fi

	local variant=$(echo -n $selection | sed -e "s/^[^\-]*-//")
	echo variant is $variant
#	check_variant $variant
	if [ $? -ne 0 ]
	then
		echo
		echo "** Invalid variant: '$variant'"
		echo "** Must be one of ${VARIANT_CHOICES[@]}"
		variant=
	fi

	if [ -z "$product" -o -z "$variant" ]
	then
		echo
		return 1
	fi

	find_complite
	export TARGET_PRODUCT=$product
	export TARGET_BUILD_VARIANT=$variant
	export TARGET_BUILD_TYPE=release

	analysis_product $product

	printconfig
}

function _lunch()
{
	local cur prev opts
	COMPREPLY=()
	cur="${COMP_WORDS[COMP_CWORD]}"
	prev="${COMP_WORDS[COMP_CWORD-1]}"

	COMPREPLY=( $(compgen -W "${LUNCH_MENU_CHOICES[*]}" -- ${cur}) )
	return 0
}

function get_make_command()
{
  echo command make
}

function make()
{
    local start_time=$(date +"%s")
    $(get_make_command) "$@"
    local ret=$?
    local end_time=$(date +"%s")
    local tdiff=$(($end_time-$start_time))
    local hours=$(($tdiff / 3600 ))
    local mins=$((($tdiff % 3600) / 60))
    local secs=$(($tdiff % 60))
    local ncolors=$(tput colors 2>/dev/null)
    if [ -n "$ncolors" ] && [ $ncolors -ge 8 ]; then
        color_failed="\e[0;31m"
        color_success="\e[0;32m"
        color_reset="\e[00m"
    else
        color_failed=""
        color_success=""
        color_reset=""
    fi
    echo
    if [ $ret -eq 0 ] ; then
        echo -n -e "${color_success}#### make completed successfully "
    else
        echo -n -e "${color_failed}#### make failed to build some targets "
    fi
    if [ $hours -gt 0 ] ; then
        printf "(%02g:%02g:%02g (hh:mm:ss))" $hours $mins $secs
    elif [ $mins -gt 0 ] ; then
        printf "(%02g:%02g (mm:ss))" $mins $secs
    elif [ $secs -gt 0 ] ; then
        printf "(%s seconds)" $secs
    fi
    echo -e " ####${color_reset}"
    echo
    return $ret
}

complete -F _lunch lunch

#cp the main Makefile
cp -u -v  $(gettop)/build/Makefile $(gettop)

# Execute the contents of any vendorsetup.sh files we can find.
for f in `test -d device && find  device -maxdepth 5 -name 'vendorsetup.sh' 2> /dev/null`
do
	echo "including $f"
	. $f
done

