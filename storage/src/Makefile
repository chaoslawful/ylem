PREFIX=`rpm --eval '%{_prefix}'`
LIBDIR=`rpm --eval '%{_libdir}'`

all::
	cd lighttpd;\
	./configure --prefix=$(PREFIX) --libdir=$(LIBDIR);\
	make

clean::
	make -C lighttpd distclean

