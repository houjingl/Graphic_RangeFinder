set_global_assignment -entity "pllIP" -library "lib_pllIP" -name IP_TOOL_NAME "altera_pll"
set_global_assignment -entity "pllIP" -library "lib_pllIP" -name IP_TOOL_VERSION "18.1"
set_global_assignment -entity "pllIP" -library "lib_pllIP" -name IP_TOOL_ENV "mwpim"
set_global_assignment -library "lib_pllIP" -name SPD_FILE [file join $::quartus(sip_path) "pllIP.spd"]

set_global_assignment -library "lib_pllIP" -name MISC_FILE [file join $::quartus(sip_path) "pllIP_sim/pllIP.vo"]
