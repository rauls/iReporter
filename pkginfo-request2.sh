# request script

PATH=/usr/sadm/bin:${PATH} # use admin utilities

MSG="The home directory for this package is 'Analyzer'. This home \
directory will be installed into '$BASEDIR' by default. "


MSG="The home directory for this package is 'Analyzer'. This home \
directory will be installed into '$BASEDIR' by default. "

PROMPT="Do you wish to install the Analyzer home directory into this location? "

HELP="A response of \"y\" will install the Analyzer home directory \
into the default location, \"n\" will allow you to specify a different location."

DIRPROMPT="Enter a new location for the Analyzer home directory"

DIRHELP="The Analyzer home directory will be installed into the specified location."

SUSPEND="Suspending installation at user request using error \
code 1."

NUBD_MSG="You have chosen to install the Analyzer home directory \
into a non-default location.  Therefore, you must update any\
applicable startup scripts to define the new home directory \
via the environment variable: IRHOME.\n"

Changed=""
Suffix="0"

#
# Determine if this product is actually installed in the working
# base directory.
#
Product_is_present () {
   if [ -d $WRKNG_BASE/EZstuf -o -d $WRKNG_BASE/HRDstuf ]; then
      return 1
   else
      return 0

   fi
}

echo

if [ ${BASEDIR} ]; then
   # This may be an old version of Solaris. In the latest Solaris
   # CLIENT_BASEDIR won't be defined yet. In older versions it is.
   if [ ${CLIENT_BASEDIR} ]; then
      LOCAL_BASE=$BASEDIR/$SUBBASE
   else # The base directory hasn't been processed yet
      LOCAL_BASE=${PKG_INSTALL_ROOT}$BASEDIR/$SUBBASE
   fi

   WRKNG_BASE=$LOCAL_BASE

   puttext "$MSG"
   result=`ckyorn -Q -d "a" -h "$HELP" -p "$PROMPT"`

   if [ $? -eq 3 ]; then
      puttext "$SUSPEND"
      exit 1
   fi

   if [ $result = "n" ]; then
      NEW_BASE=`ckpath -yw -h "$DIRHELP" -p "$DIRPROMPT"`
      echo "BASEDIR=$NEW_BASE" >> $1;
   elif [ $result="a" ]; then
      exit 0
   else
      exit 1
   fi
   puttext "$NUBD_MSG"
fi

#echo BASEDIR=$BASEDIR

exit 0
