#!/bin/bash
discord > /dev/null 2>&1 &
sleep 2
wmctrl -r "discord" -e 0,0,1440,1440,1275
