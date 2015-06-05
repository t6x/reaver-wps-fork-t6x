#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=reaver-wps-fork-t6x
PKG_REV:=Big_endian
PKG_VERSION:=$(PKG_REV)
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/t6x/reaver-wps-fork-t6x.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_PROTO:=git

include $(INCLUDE_DIR)/package.mk

define Package/reaver
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=wireless
  TITLE:=Efficient online and offline brute force attack against Wifi Protected Setup
  URL:=https://github.com/t6x/reaver-wps-fork-t6x
  DEPENDS:=+libpcap +libsqlite3
endef

define Package/reaver/description
  Reaver targets the external registrar functionality mandated by the WiFi
  Protected Setup specification.
  Access points will provide authenticated registrars with their current
  wireless configuration (including the WPA PSK), and also accept a new
  configuration from the registrar.
  New implementations consist of the PixieDust attack and many more.
endef

CONFIGURE_PATH:=src

MAKE_PATH:=src

TARGET_CFLAGS+=$(TARGET_CPPFLAGS)

define Package/reaver/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/{reaver,wash} $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/reaver
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/reaver.db $(1)/etc/reaver/
endef

$(eval $(call BuildPackage,reaver)) 
