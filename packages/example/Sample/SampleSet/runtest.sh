#!/bin/bash

#---------------------#
PREFIX="-en \033["

CURSOR_HIDE="$PREFIX?25l"
CURSOR_SHOW="$PREFIX?25h"
WORD_RED=$PREFIX"31m"
WORD_GREEN=$PREFIX"32m"
WORD_YELLOW=$PREFIX"33m"
WORD_BLUE=$PREFIX"34m"
WORD_WHITE=$PREFIX"37m"
BLANK=$PREFIX"8m"
UNBLANK=$PREFIX"7m"
BUTTOM_LINE=$PREFIX"4m"
#---------------------#

MENU_WIDTH=50
WORK_DIR=$(pwd)

head_info(){
	cat <<EOF > $WORK_DIR/menu_config
#menu_main
name=phoenix test
EOF
}
#Automatic test=all_test

menu_file(){
	echo $1"="$2 >> $WORK_DIR/menu_config
}

function cut_file(){
	filename=$1
	filename=${filename##./}
	itemname=${filename##*/}
	itemname=${itemname%.*}
}

menu_dir(){
	local item
	local file
	local cur_dir=$1

	echo "#menu_"$cur_dir >> $WORK_DIR/menu_config
	echo "name="$cur_dir "test" >> $WORK_DIR/menu_config

	((ls $cur_dir/*.sh -1 | grep -r "test_") & \
	(ls $cur_dir/*.sh -1 | grep -r "Demo") & \
	(ls $cur_dir/*.sh -1 | grep -r "Test")) | \
	{
		while read line; do
			file=${line##*/}
			item=${file##*_}
			item=${item%.*}
			menu_file $item $WORK_DIR/$cur_dir/$file
		done
	}
	echo "exit=menu_main" >> $WORK_DIR/menu_config
}

make_menuconfig(){
	if test -d $filename
	then
		menu_file $itemname" --->" "menu_"$itemname
	else
		((echo "$filename" | grep -q "test_") || \
			(echo "$filename" | grep -q "Test") || \
			(echo "$filename" | grep -q "Demo")) && \
			{
				menu_file $itemname $WORK_DIR/$filename
			}
		fi
}

function loop_menuconfig(){
	for file in ./*
	do
		cut_file $file
		make_menuconfig ${filename}
	done
	menu_file "exit" "all_exit"
}

function loop_dirconfig(){
	for file in ./*
	do
		cut_file $file
		if test -d $filename
		then
			menu_dir ${filename##*_}
			cd ${filename}
			for file1 in ./*
			do
				cut_file $file1
				if test -d $filename
				then
					make_menuconfig $filename
				fi
			done
			for file1 in ./*
			do
				cut_file $file1
				if test -d $filename
				then
					loop_dirconfig $filename
				fi
			done
			cd ..
		fi
	done
}


blank_space() {
	local space=
	for((i=1;i<=$1;i++));do
		space=$space' '
	done
	echo "$space"
	return 0
}

test_all() {
	local result=
	local item
	local tmp
	((find -name "*.sh" | grep -r "test_") & \
	(find -name "*.sh" | grep -r "Demo") & \
	(find -name "*.sh" | grep -r "Test")) | \
	{
		while read line; do
			item=${line:5}
			((tmp=${#item}-3))
			item=${item:0:$tmp}
			((tmp=20-tmp))
			tmp="$(blank_space tmp)"
			if [ $line != 'test_exit.sh' ]
			then
				$PRETEST_DIR/$line #&>/dev/null
				case "$?" in
				'0')
					result=""$result"  "$item""${tmp}"|\ttest OK!\n"
					;;
				'2')
					result=""$result"  "$item""${tmp}"|\ttest manual\n"
					;;
				*)
					result=""$result"  "$item""${tmp}"|\ttest Failed!\n"
					;;
				esac
			fi
		done
		clear
		echo
		echo "-------------- Test result -------------"
		echo
		echo -e "$result"
	}
}


menu_echo() {
	for((i=1;i<=$2;i++));do
		echo -en "$1"
	done
	return 0
}

menu() {
	local para=$1
	shift
	local content=$*
	local left=
	local tmp=0
	((tmp=${#content}%2))
	[ "$tmp" = 1 ] && content="$content "

	[ "$para" = "title" ] && {
		((left=($MENU_WIDTH-${#content})/2))
		menu_echo " " 1
		menu_echo - left
		echo -en "$content"
		menu_echo - left
		echo -en "\n|"
		menu_echo " " MENU_WIDTH
		echo -en "|\n"
	}
	[ "$para" = "list" ] && {
		((left=$MENU_WIDTH-${#content}-9))
		echo -n "|"
		menu_echo " " 5
		echo -en "$current_list_num:"
		[[ current_list_num -gt 9 ]] && menu_echo " " 1 || menu_echo " " 2
		echo -n "$content"
		menu_echo " " left
		echo -en "|\n"
		((current_list_num++))
	}
	return 0
}

make_menu() {
	current_list_num=1
	local get_it=0

	while read line; do
		[ ${#line} != 0 ] && {
			if [ $get_it -eq 0 ]; then
				[ ${line:0:1} = '#' -a "$1" = "${line:1}" ] && get_it=1
			else
				[ "${line:0:1}" = "#" ] && break
				[ "${line%%=*}" = "name" ] && menu title "${line#*=}" || menu list "${line%%=*}"
			fi
		}
	done < "./menu_config"
	echo -en "|"
	menu_echo " " MENU_WIDTH
	echo -en "|\n"
	menu_echo " " 1
	menu_echo "-" MENU_WIDTH
	echo -en "\n"
	return 0
}

get_final() {
	current_list_num=1
	func_return=""
	local get_it=0
	while read line; do
		[ ${#line} != 0 ] && {
			if [ $get_it -eq 0 ]; then
				[ ${line:0:1} = '#' ] && [ "$1" = ${line:1} ] && get_it=1
			else
				[ "${line:0:1}" = "#" ] && break
				[ "${line%%=*}" != "name" ] && {
					[ $current_list_num -eq "$2" ] && func_return="${line#*=}" \
					&& break
					((current_list_num++))
				}
			fi
		}
	done < "./menu_config"
}

menu_handle() {
	local menu_next=
	local cmd=
	local item
	local tmp
	if [ "${func_return:0:4}" = "menu" ]; then
		menu_next=$func_return
		echo next menu: $menu_next
		menu_current=$menu_next
	else
		cmd=$func_return
		if [ "$cmd" = "all_exit" ]; then
			exit
		elif [ "$cmd" = "all_test" ]; then
			echo "  Testing ......"
			test_all
		else
			[ -f $PRETEST_DIR/$cmd ] && {
				item=${cmd:5}
				((tmp=${#item}-3))
				item=${item:0:$tmp}
				$PRETEST_DIR/$cmd #&>/dev/null
				case "$?" in
				'0')
					echo "  $item test OK! "
					;;
				'2')
					;;
				*)
					echo "  $item test failed! "
					;;
				esac
			}
		fi
		read -p "  press ENTER continue"
	fi
}

menu_get_value() {
	local list_num=
	clear
	make_menu "$1"
	echo -en "  please select one:  "
	read list_num
	expr $list_num + 0 &>/dev/null
	[ $? = 0 ] && get_final "$1" $list_num || return -1
}

#main

#sleep 2
menu_current=menu_main
head_info
loop_menuconfig
loop_dirconfig

while true; do
	menu_get_value $menu_current
	[ $? = 0 ] && menu_handle
done

#main end

exit
