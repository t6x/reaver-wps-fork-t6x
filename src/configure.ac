AC_INIT(reaver, 1.5.3)

AC_PROG_CC
AC_LANG(C)

CFLAGS="-Wall $CFLAGS"
LDFLAGS="-lm -lpcap $LDFLAGS"

AC_CHECK_LIB(pcap, pcap_open_live, [], [echo "error: pcap library not found!"; exit -1])
AC_CHECK_HEADERS([stdlib.h stdint.h string.h],[],[echo "error: missing standard header files" && exit -1])
AC_CHECK_HEADERS([pcap.h],[],[echo "error: missing pcap header files" && exit -1])

AC_ARG_ENABLE(savetocurrent,
AC_HELP_STRING([--enable-savetocurrent],
  [saves the current session file to the directory reaver was started from])
, [ CFLAGS="$CFLAGS -DSAVETOCURRENT" ])

cp confdefs.h config.h

AC_SUBST(target, $PACKAGE_NAME)
AC_OUTPUT(Makefile)
