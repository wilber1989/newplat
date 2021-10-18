include $(TOPDIR)/rules.mk

PKG_NAME:=newplat
PKG_RELEASE:=4.1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/newplat
  SECTION:=wthink
  CATEGORY:=Wthink packages
  DEPENDS:= +libuuid +libmosquitto +libpthread +libcjson +libubox +libuci +libubus 
  TITLE:=newplat
endef

define Package/newplat/description
	5G UBOX UAV newplat handle
endef

TARGET_CXXFLAGS += -I$(PKG_BUILD_DIR)
TARGET_CXXFLAGS += -L$(PKG_BUILD_DIR)/include

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/newplat/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/newplat $(1)/usr/sbin/newplat
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,newplat))
