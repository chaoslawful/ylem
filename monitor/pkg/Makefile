PWD := $(shell pwd)
RPM_ROOT := $(PWD)/build
RPM_SOURCE_ROOT := $(RPM_ROOT)/SOURCES
RPM_SPEC_ROOT := $(RPM_ROOT)/SPECS
RPMBUILD_OPT := -bb --clean --rmspec --rmsource --define="_topdir $(RPM_ROOT)"
PKGS := \
	   ylm_clmon_conf \
	   ylm_clmon \
	   ylm_dbmon \
	   ylm_stmon

all: $(PKGS)
	find $(RPM_ROOT) -name '*.rpm' -exec mv '{}' $(PWD) ';'

prepare:
	mkdir -p $(RPM_ROOT)/BUILD
	mkdir -p $(RPM_ROOT)/RPMS
	mkdir -p $(RPM_ROOT)/SRPMS
	mkdir -p $(RPM_ROOT)/SPECS
	mkdir -p $(RPM_ROOT)/SOURCES

ylm_clmon_conf: prepare
	cp -f ../conf/dbmon_boxes $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_clmon: prepare
	cp -f ../conf/ylm_clmon-start $(RPM_SOURCE_ROOT)/
	cp -f ../conf/ylm_clmon-stop $(RPM_SOURCE_ROOT)/
	cp -f ../src/cluster_mon/ylm_clmon $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_dbmon: prepare
	cp -f ../conf/ylm_dbmon-start $(RPM_SOURCE_ROOT)/
	cp -f ../conf/ylm_dbmon-stop $(RPM_SOURCE_ROOT)/
	cp -f ../src/database_mon/ylm_dbmon $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

ylm_stmon: prepare
	cp -f ../conf/ylm_stmon-start $(RPM_SOURCE_ROOT)/
	cp -f ../conf/ylm_stmon-stop $(RPM_SOURCE_ROOT)/
	cp -f ../src/storage_mon/ylm_stmon $(RPM_SOURCE_ROOT)/
	cp -rf ../src/storage_mon/lib/Mogstored $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

clean:
	-rm -rf $(RPM_ROOT) *.rpm

.PHONY: all prepare clean

