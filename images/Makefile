prefix := /usr

BASENAME = itu-r-bt1729-colorbar-
BASERES = 3200x1800
BASEFILE = $(BASENAME)$(BASERES).png

RESOLUTIONS24 = 1920x1080 1360x768 1280x720 1024x768 800x600 480x272 320x200
RESOLUTIONS32 = 1280x1024

all:
	for x in $(RESOLUTIONS24); do \
		echo "Creating $(BASENAME)$$x.raw.xz" ; \
		convert $(BASEFILE) -resize $$x\! $(BASENAME)$$x.png ; \
		./png2fb.py $(BASENAME)$$x.png $(BASENAME)$$x.raw ; \
		rm -f $(BASENAME)$$x.raw.xz ; \
		xz $(BASENAME)$$x.raw ; \
	done
	for x in $(RESOLUTIONS32); do \
		echo "Creating $(BASENAME)$$x-32.raw.xz" ; \
		convert $(BASEFILE) -resize $$x\! $(BASENAME)$$x-32.png ; \
		./png2fb.py $(BASENAME)$$x-32.png $(BASENAME)$$x-32.raw -32 ; \
		rm -f $(BASENAME)$$x-32.raw.xz ; \
		xz $(BASENAME)$$x-32.raw ; \
	done

install:
	install -m 755 -d $(DESTDIR)$(prefix)/share/beagle-tester
	for x in $(RESOLUTIONS24); do \
		echo "Installing $(BASENAME)$$x.raw.xz" ; \
		install -m 644 $(BASENAME)$$x.raw.xz $(DESTDIR)$(prefix)/share/beagle-tester ; \
	done
	for x in $(RESOLUTIONS32); do \
		echo "Installing $(BASENAME)$$x-32.raw.xz" ; \
		install -m 644 $(BASENAME)$$x-32.raw.xz $(DESTDIR)$(prefix)/share/beagle-tester ; \
	done

