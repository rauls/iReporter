#!/bin/sh

echo Creating soft-link in /usr/bin/ireporter to $BASEDIR/$NAME/bin/ireporter

rm -f /usr/bin/ireporter
ln -s $BASEDIR/$NAME/bin/ireporter /usr/bin/ireporter

echo --------------------------------------------------------
echo You may now use iReporter using 'ireporter'
echo
echo An example of how to produce a report from a log file:
echo 'ireporter -l logfile.log -o reportlocation/index.html'
echo 
echo for more info issue 'ireporter -h' command.

if [ $BASEDIR != "/opt" ]; then
   echo Warning: Make sure IRHOME is set to $BASEDIR/$NAME
fi

echo
echo --------------------------------------------------------
echo

echo Done.
echo
