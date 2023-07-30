#!/bin/bash
gnome-terminal --working-directory=./Workspace
sleep .1
wmctrl -r "superfortress" -e 0,4000,1920,1140,900
