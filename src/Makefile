-include config.mak

LIBWPS_DIR=libwps
INC=-Ilibwps -I. -Ilwe

DISABLED_WARNINGS= \
 -Wno-unused-variable \
 -Wno-unused-but-set-variable \
 -Wno-unused-function \
 -Wno-pointer-sign

CFLAGS=-DCONF_DIR='"$(CONFDIR)"' $(CFLAGS_USER) $(DISABLED_WARNINGS)
CFLAGS += $(INC)
CFLAGS += -DCONFIG_IPV6

UTILS_OBJS= \
        utils/base64.o \
        utils/common.o \
        utils/ip_addr.o \
        utils/radiotap.o \
        utils/trace.o \
        utils/uuid.o \
        utils/wpa_debug.o \
        utils/wpabuf.o \
	utils/os_unix.o \
	utils/eloop.o


WPS_OBJS=wps/wps_attr_build.o wps/wps_attr_parse.o wps/wps_attr_process.o \
wps/wps.o wps/wps_common.o wps/wps_dev_attr.o wps/wps_enrollee.o \
wps/wps_registrar.o wps/wps_ufd.o

LWE_OBJS=lwe/iwlib.o

LIB_OBJS=libwps/libwps.o $(WPS_OBJS) $(UTILS_OBJS) \
	crypto/libcrypto.a tls/libtls.a $(LWE_OBJS)

WASH_OBJS=globule.o init.o misc.o 80211.o iface.o \
 builder.o crc.o pixie.o wpsmon.o
REAVER_OBJS=argsparser.o globule.o init.o sigint.o iface.o sigalrm.o \
 misc.o cracker.o session.o send.o pins.o 80211.o exchange.o builder.o \
 keys.o crc.o pixie.o wpscrack.o

# Version of the Wireless Tools
WT_VERSION := $(shell sed -ne "/WT_VERSION/{s:\([^0-9]*\)::;p;q;}" < lwe/iwlib.h )
# Version of Wireless Extensions.
WE_VERSION := $(shell sed -ne "/WE_VERSION/{s:\([^0-9]*\)::;p;q;}" < lwe/iwlib.h )
# Always use local header for wireless extensions
WEXT_HEADER = lwe/wireless.$(WE_VERSION).h


all: wash reaver

$(WPS_OBJS): CFLAGS+=-I. -Iutils

wash: $(WASH_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(INC) $(WASH_OBJS) $(LIB_OBJS) $(LDFLAGS) -o wash

reaver: $(REAVER_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(INC) $(REAVER_OBJS) $(LIB_OBJS) $(LDFLAGS) -o reaver

crypto/libcrypto.a: tls/libtls.a
	(cd crypto && make)

tls/libtls.a:
	(cd tls && make)

lwe/wireless.h: $(WEXT_HEADER)
	cp $(WEXT_HEADER) lwe/wireless.h

$(WASH_OBJS) $(REAVER_OBJS) $(LWE_OBJS): lwe/wireless.h

install: wash reaver
	install -D -m 755 wash $(DESTDIR)$(exec_prefix)/bin/wash
	install -D -m 755 reaver $(DESTDIR)$(exec_prefix)/bin/reaver
	@# create directory used to store *.wpc files;
	@# if not found defaults to "." i.e. current dir
	install -d $(DESTDIR)$(CONFDIR)

clean:
	rm -f *~ *.o reaver wash
	rm -rf *.cache config.status config.log
	rm -f $(LIB_OBJS)
	(cd utils && make clean)
	(cd tls && make clean)
	(cd crypto && make clean)
	rm -f lwe/wireless.h

cleanbin: 
	rm -f @bindir@/reaver @bindir@/wash @bindir@/walsh

distclean: cleanbin clean
	rm -rf $(CONFDIR)

.PHONY: all clean cleanbin distclean install

