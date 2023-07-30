#!/bin/bash
google-chrome --profile-directory=Default https://chat.openai.com > /dev/null 2>&1 &
sleep .1
wmctrl -r "chrome" -e 0,0,-20,1480,1290
