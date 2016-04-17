#!/bin/bash

OUT_PIPE="/tmp/led_fifo_cmd"
IN_PIPE="/tmp/led_fifo_resp"
TOUT=2
ERR_SIGTERM=143

usage(){
echo "usage:
	
control commands:
$0 on|off
$0 color red|green|blue
$0 rate 0|1|2|3|4|5

get state commands:
$0 getstate
$0 getcolor
$0 getrate

get whole state:
$0
"

	exit 1
}

failed(){
	echo -e "command failed!\n"
	usage
}

sendcmd(){
	echo "$1">"$OUT_PIPE"
	local resp=`head -n 1 "$IN_PIPE"`
	echo "$resp"
}

get_state(){
	local resp=`sendcmd "get-led-state"`
	echo "$resp"
}

get_color(){
	local resp=`sendcmd "get-led-color"`
	echo "$resp"
}

get_rate(){
	local resp=`sendcmd "get-led-rate"`
	echo "$resp"
}

set_state(){
	local resp=`sendcmd "set-led-state $1"`
	echo "$resp"
}

set_color(){
	local resp=`sendcmd "set-led-color $1"`
	echo "$resp"
}

set_rate(){
	local resp=`sendcmd "set-led-rate $1"`
	echo "$resp"
}

extract_info(){
	echo "$1" | sed "s/\s*OK\s*//"
}

get_whole_state(){
	local resp=""
	local info=""
	
	resp=`get_state`
	if [[ ! "$resp" =~ .*OK.* ]]; then
		failed
	fi
	info+="state: "`extract_info "$resp"`"\n"
	
	resp=`get_color`
	if [[ ! "$resp" =~ .*OK.* ]]; then
		failed
	fi
	info+="color: "`extract_info "$resp"`"\n"
	
	resp=`get_rate`
	if [[ ! "$resp" =~ .*OK.* ]]; then
		failed
	fi
	info+="rate: "`extract_info "$resp"`	
	
	echo -e "$info"
}

main(){
	local resp=""
	local info=""

	if [ -z "$1" ]; then
		get_whole_state
		exit 0
	else
		case "$1" in
		 "on" )
			resp=`set_state "on"`
		 ;;

		 "off" )
			resp=`set_state "off"`
		 ;;
		 
		 "color" )
			if [ -z "$2" ]; then
				usage
			else
				resp=`set_color "$2"`
			fi
		 ;;
		 
		 "rate" )
			if [ -z "$2" ]; then
				usage
			else
				resp=`set_rate "$2"`
			fi
		 ;;

		"getstate" )
			resp=`get_state`
		;;
		
		"getcolor" )
			resp=`get_color`
		;;
		
		"getrate" )
			resp=`get_rate`
		;;
		 
		 *)
			usage
		 ;;
		esac				
	fi
	
	if [[ "$resp" =~ .*OK.* ]]; then
		info=`extract_info "$resp"`
		if [ "$info" != "" ]; then
			echo "$info"
		fi
	else
		failed
	fi
}



if [ "$1" == "-e" ]; then
	shift
	(	
		flock -x 200
		main $@
	) 200>"$IN_PIPE"
else
	cmd="$0 -e $@"
	timeout --preserve-status $TOUT bash -c "$cmd"
	ret=$?
	if [ $ret == $ERR_SIGTERM ]; then
		echo "error: timed out!"
	fi
	exit $ret
fi





	