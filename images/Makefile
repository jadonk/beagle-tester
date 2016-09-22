prefix := /usr

BASENAME = itu-r-bt1729-colorbar-
BASERES = 3200x1800
BASEFILE = $(BASENAME)$(BASERES).png

RESOLUTIONS = 1920x1080 1360x768 1280x720 800x600

all:
	for x in $(RESOLUTIONS); do \
		echo "Creating $(BASENAME)$$x.raw.xz" ; \
		convert $(BASEFILE) -resize $$x $(BASENAME)$$x.png ; \
		./png2fb.py $(BASENAME)$$x.png $(BASENAME)$$x.raw ; \
		rm -f $(BASENAME)$$x.raw.xz ; \
		xz $(BASENAME)$$x.raw ; \
	done

install:
	install -m 755 -d $(DESTDIR)$(prefix)/share/beagle-tester
	for x in $(RESOLUTIONS); do \
		echo "Installing $(BASENAME)$$x.raw.xz" ; \
		install -m 644 $(BASENAME)$$x.raw.xz $(DESTDIR)$(prefix)/share/beagle-tester ; \
	done

