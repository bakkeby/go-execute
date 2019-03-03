#!/bin/bash
# Copyright (c) 2019, Stein Gunnar Bakkeby, all rights reserved.
#
# Optional configuration settings for the gx script. There is no
# reason to source this unless you actually intend on changing
# anything.
#
# This is more meant as a reference documenting the default values
# used by the script.

typeset -Ag _GXSETTINGS

# Print settings
_GXSETTINGS[print_cmd]=0; # print the command before running it
_GXSETTINGS[help_format]="  %-31s%s\n"; # controls go -h layout
_GXSETTINGS[list_format]="%-17s%s%-10s\n"; # controls gx -l layout
_GXSETTINGS[symbol_tmp]="ᴛ"
_GXSETTINGS[symbol_pin]="🖈"
_GXSETTINGS[symbol_dir]="‒"

# Evaluation settings
_GXSETTINGS[filestore]=${HOME:-~}/.gx.db
_GXSETTINGS[post_cd_cmd]="ls -a --color=auto"; # command to run after changing directory