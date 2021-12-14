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
test -z "$PKGCONFIG" && PKGCONFIG=pkg-config
AC_ARG_ENABLE(libnl3,
AC_HELP_STRING([--enable-libnl3],
  [compiles with libnl3 instead of wext])
, [
    if $PKGCONFIG --cflags libnl-3.0 libnl-genl-3.0 >/dev/null 2>&1 ; then
      LIBNL_CLAGS="$($PKGCONFIG --cflags libnl-3.0 libnl-genl-3.0) -DLIBNL3"
      LIBNL_LDFLAGS="$($PKGCONFIG --libs libnl-3.0 libnl-genl-3.0)"
    else
      AC_MSG_ERROR([failed to query $PKGCONFIG --cflags libnl-3.0 libnl-genl-3.0, make sure to have both libraries installed!])
    fi
])
AC_ARG_ENABLE(libnl-tiny,
AC_HELP_STRING([--enable-libnl-tiny],
  [compiles with libnl-tiny instead of wext])
, [
    if $PKGCONFIG --cflags libnl-tiny >/dev/null 2>&1 ; then
      LIBNL_CLAGS="$($PKGCONFIG --cflags libnl-tiny) -DLIBNL3"
      LIBNL_LDFLAGS="$($PKGCONFIG --libs libnl-tiny)"
    else
      AC_MSG_ERROR([failed to query $PKGCONFIG --cflags libnl-tiny, make sure to the library installed!])
    fi
])

AC_SUBST(LIBNL_CFLAGS)
AC_SUBST(LIBNL_LDFLAGS)

DESIRED_FLAGS="-Werror-unknown-warning-option -Wno-unused-but-set-variable"
for flag in $DESIRED_FLAGS; do
  AS_COMPILER_FLAG([$flag], [CFLAGS="$CFLAGS $flag"])
done

cp confdefs.h config.h

AC_SUBST(target, $PACKAGE_NAME)
AC_OUTPUT()
