
DOCS = README CHANGES BUGS

.PHONY: all install clean

all:
	make -C build

install:
	cp -a $(DOCS) debian
	make -C build install

clean:
	make -C build clean
	(cd debian; \
		rm -rf $(DOCS))

distclean: clean

