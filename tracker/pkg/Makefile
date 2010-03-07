PWD := $(shell pwd)
RPM_ROOT := $(PWD)/build
RPM_SOURCE_ROOT := $(RPM_ROOT)/SOURCES
RPM_SPEC_ROOT := $(RPM_ROOT)/SPECS
RPMBUILD_OPT := -bb --clean --rmspec --rmsource --define="_topdir $(RPM_ROOT)"
PKGS := \
	ylm_tracker \
	ylm_tracker_conf \
	ylm_tracker_util \
	ylm_tracker_util_conf

all: $(PKGS)
	find $(RPM_ROOT) -name '*.rpm' -exec mv '{}' $(PWD) ';'

prepare:
	mkdir -p $(RPM_ROOT)/BUILD
	mkdir -p $(RPM_ROOT)/RPMS
	mkdir -p $(RPM_ROOT)/SRPMS
	mkdir -p $(RPM_ROOT)/SPECS
	mkdir -p $(RPM_ROOT)/SOURCES

ylm_tracker: prepare
	cp ../conf/ylm_tracker-start $(RPM_SOURCE_ROOT)/
	cp ../conf/ylm_tracker-stop $(RPM_SOURCE_ROOT)/
	cp -rf ../src/server/lib $(RPM_SOURCE_ROOT)/
	cp ../src/server/ylm_tracker $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_tracker_conf: prepare
	cp ../conf/tracker.conf $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_tracker_util: prepare
	cp ../src/utils/mogdbsetup $(RPM_SOURCE_ROOT)/
	cp ../src/utils/mogadm $(RPM_SOURCE_ROOT)/
	cp ../src/utils/mogtool $(RPM_SOURCE_ROOT)/
	cp -rf ../src/utils/lib $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_tracker_util_conf: prepare
	cp ../conf/admin.conf $(RPM_SOURCE_ROOT)/
	cp ../conf/tools.conf $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

clean:
	-rm -rf $(RPM_ROOT) *.rpm

.PHONY: all prepare clean

