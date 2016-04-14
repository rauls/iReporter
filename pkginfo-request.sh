# request script

PATH=/usr/sadm/bin:${PATH} # use admin utilities

#MSG="Analyzer will be installed into '$BASEDIR/$NAME' by default \
#You may override the default base directory of '$BASEDIR' if you wish. "

PROMPT="Do you wish to install Analyzer into its default base directory? "

HELP="A response of \"y\" will install the Analyzer home directory \
into the base directory '$BASEDIR'.  A response of \"n\" will \
allow you to specify a different base directory."

DIRPROMPT="Enter you preferred base directory "

DIRHELP="The Analyzer home directory will be installed beneath the specified base directory."

SUSPEND="Suspending installation at user request using error \
code 1."

NUBD_MSG="You have chosen to install the Analyzer home directory \
into a non-default base directory.  Therefore, you must update any \
applicable startup scripts with the definition of new home directory \
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
