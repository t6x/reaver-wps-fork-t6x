define(VERSION_STRING, esyscmd(cat VERSION | tr -d '\n'))
AC_INIT(reaver, VERSION_STRING)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([config.mak])
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

DESIRED_FLAGS="-Werror-unknown-warning-option -Wno-unused-but-set-variable"
for flag in $DESIRED_FLAGS; do
  AS_COMPILER_FLAG([$flag], [CFLAGS="$CFLAGS $flag"])
done

cp confdefs.h config.h

AC_SUBST(target, $PACKAGE_NAME)
AC_OUTPUT()
