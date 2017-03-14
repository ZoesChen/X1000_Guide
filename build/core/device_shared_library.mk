LOCAL_EXT_NAME:=.so

LOCAL_OUT_INSTALL_DIR:=$(OUT_DEVICE_SHARED_DIR)

include $(BUILD_SYSTEM)/base_ruler.mk

$(LOCAL_INSTALL_MODULE):$(LOCAL_OUT_MODULE_BUILD)
	$(hide) $(call host-mkdir,$(dir $@))
	$(hide) $(TARGET_STRIP) -o $@ $<

$(LOCAL_OUT_MODULE_BUILD):$(LOCAL_MODULE_OBJS)
	$(tranforms_device_o_to_so)
