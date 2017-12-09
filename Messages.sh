#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT *.cxx rc.cpp -o $podir/liquidshell.pot
rm -f rc.cpp
