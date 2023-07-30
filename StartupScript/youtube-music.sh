#!/bin/bash
# vim the xxx.desktop file, the command to launch the app should be on the line that starts with `Exec=`
google-chrome --profile-directory=Default --app-id=cinhimbnkkaeohfgghhklpknlkffjgod > /dev/null 2>&1 &
sleep .1
wmctrl -r "music" -e 0,4000,620,1124,1120
