PWD := $(shell pwd)
RPM_ROOT := $(PWD)/build
RPM_SOURCE_ROOT := $(RPM_ROOT)/SOURCES
RPM_SPEC_ROOT := $(RPM_ROOT)/SPECS
RPMBUILD_OPT := -bb --clean --rmspec --rmsource --define="_topdir $(RPM_ROOT)"
PKGS := \
	ylm_storage \
	lighttpd_storage \
	lighttpd_storage_conf

all: $(PKGS)
	find $(RPM_ROOT) -name '*.rpm' -exec mv '{}' $(PWD) ';'

prepare:
	mkdir -p $(RPM_ROOT)/BUILD
	mkdir -p $(RPM_ROOT)/RPMS
	mkdir -p $(RPM_ROOT)/SRPMS
	mkdir -p $(RPM_ROOT)/SPECS
	mkdir -p $(RPM_ROOT)/SOURCES

ylm_storage: prepare
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

lighttpd_storage: prepare
	cp -r ../src/lighttpd/src/.libs $(RPM_SOURCE_ROOT)/libs
	cp ../src/lighttpd/src/spawn-fcgi $(RPM_SOURCE_ROOT)/
	cp ../src/lighttpd/src/lighttpd $(RPM_SOURCE_ROOT)/
	cp ../src/lighttpd/src/lighttpd-angel $(RPM_SOURCE_ROOT)/
	cp ../src/lighttpd/doc/rc.lighttpd.redhat $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

lighttpd_storage_conf: prepare
	cp ../conf/lighttpd.conf $(RPM_SOURCE_ROOT)/
	cp ../conf/index.html $(RPM_SOURCE_ROOT)/
	cp ../conf/log_rotate.conf $(RPM_SOURCE_ROOT)/
	cp -f $@.spec $(RPM_SPEC_ROOT)/
	rpmbuild $(RPMBUILD_OPT) $(RPM_SPEC_ROOT)/$@.spec

clean:
	-rm -rf $(RPM_ROOT) *.rpm

.PHONY: all prepare clean

