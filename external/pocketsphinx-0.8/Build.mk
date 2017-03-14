LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE :=pocketsphinx-lib-build

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_GEN_SHARED_FILES:= .install/lib/libpocketsphinx.la \
				.install/lib/libpocketsphinx.so \
				.install/lib/libpocketsphinx.so.1 \
				.install/lib/libpocketsphinx.so.1.1.0

LOCAL_MODULE_CONFIG_FILES:=Makefile
LOCAL_MODULE_CONFIG=touch *;./autogen.sh --prefix=$(TOP_DIR)/$(LOCAL_PATH)/.install --host=$(DEVICE_COMPILER_PREFIX) --enable-static=no --without-python CFLAGS="-I$(ABS_DEVICE_INCLUDE_DIR)" LDFLAGS="-L$(ABS_DEVICE_SHARED_DIR) -Wl,-rpath -Wl,$(ABS_DEVICE_SHARED_DIR)"

LOCAL_MODULE_COMPILE :=make ;make install
LOCAL_MODULE_COMPILE_CLEAN :=make distclean; rm .install -rf
LOCAL_DEPANNER_MODULES := sphinxbase-lib-build
include $(BUILD_THIRDPART)

#-----------------
# copy file
#
include $(CLEAR_VARS)
LOCAL_MODULE :=pocketsphinx-lib
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_FS_BUILD)

LOCAL_COPY_FILES :=  \
	usr/sbin/pocketsphinx_continuous:.install/bin/pocketsphinx_continuous \
	usr/lib/libsndfile.so.1:util/libsndfile.so.1 \
	pocketsphinx/7010.dic:util/7010.dic \
	pocketsphinx/7010.lm:util/7010.lm \
	pocketsphinx/tdt_sc_8k/feat.params:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/feat.params \
	pocketsphinx/tdt_sc_8k/mdef:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/mdef \
	pocketsphinx/tdt_sc_8k/means:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/means \
	pocketsphinx/tdt_sc_8k/noisedict:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/noisedict \
	pocketsphinx/tdt_sc_8k/sendump:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/sendump \
	pocketsphinx/tdt_sc_8k/transition_matrices:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/transition_matrices \
	pocketsphinx/tdt_sc_8k/variances:.install/share/pocketsphinx/model/hmm/zh/tdt_sc_8k/variances

LOCAL_DEPANNER_MODULES :=pocketsphinx-lib-build
include $(BUILD_MULTI_COPY)




