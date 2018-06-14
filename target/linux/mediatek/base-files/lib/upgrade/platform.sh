platform_do_upgrade() {                 
	default_do_upgrade "$ARGV"                                               
}                                                                                

PART_NAME=firmware

platform_check_image() {                                                         
	local board=$(board_name)                                                
	local magic="$(get_magic_long "$1")"                                     

	[ "$#" -gt 1 ] && return 1                                               

	case "$board" in                                                       
	bananapi,bpi-r2)                                                       
		[ "$magic" != "27051956" ] && {   
			echo "Invalid image type."
			return 1                                     
		}                                                    
		return 0                                             
		;;                                                   

	*)                                                           
		echo "Sysupgrade is not supported on your board yet."
		return 1                                             
		;;                                
	esac                                      

	return 0                                                                                         
}                   
