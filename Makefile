prefix := /usr
CC := gcc
MAKE := make
RM := rm
INSTALL := install
GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)

all: beagle-tester

beagle-tester: beagle-tester.c
	#$(CC) $(CFLAGS_FOR_BUILD) -W -Wall -Wwrite-strings -O3 -o beagle-tester beagle-tester.c -lroboticscape
	$(CC) -DVERSION=\"${GIT_VERSION}\" $(CFLAGS_FOR_BUILD) -W -Wall -Wwrite-strings -O3 -o beagle-tester beagle-tester.c

images:
	$(MAKE) -C images

clean:
	$(RM) beagle-tester

install:
	$(INSTALL) -m 755 -d $(DESTDIR)$(prefix)/sbin
	$(INSTALL) -m 700 beagle-tester $(DESTDIR)$(prefix)/sbin
	$(INSTALL) -m 744 bb-connect-ap $(DESTDIR)$(prefix)/sbin
	$(INSTALL) -m 744 beagle-tester-open.sh $(DESTDIR)$(prefix)/sbin
	$(INSTALL) -m 744 beagle-tester-close.sh $(DESTDIR)$(prefix)/sbin
	$(INSTALL) -m 755 -d $(DESTDIR)/lib/systemd/system
	$(INSTALL) -m 644 beagle-tester.service $(DESTDIR)/lib/systemd/system
	$(INSTALL) -m 755 -d $(DESTDIR)/etc/udev/rules.d
	$(INSTALL) -m 644 beagle-tester.rules $(DESTDIR)/etc/udev/rules.d
	$(INSTALL) -m 755 -d $(DESTDIR)/lib/firmware
	$(INSTALL) -m 644 buzz.out $(DESTDIR)/lib/firmware/am335x-pru0-fw
	$(MAKE) -C images -s install
	#systemctl stop beagle-tester.service || true
	#systemctl daemon-reload || true
	#systemctl enable beagle-tester.service || true

start: install
	systemctl restart beagle-tester.service
