prefix := /usr
CC := gcc
MAKE := make
RM := rm
INSTALL := install
GIT_VERSION := $(shell git describe --abbrev=6 --dirty --always --tags)
SRC := $(wildcard src/*.c)
INC := $(wildcard include/*.h)
OBJS := ${patsubst %.c,%.o,${SRC}}
CFLAGS := $(CFLAGS_FOR_BUILD) -O3 -W -Wall -Wwrite-strings -I./include

all: beagle-tester

beagle-tester: $(SRC) $(INC)
	$(CC) -DVERSION=\"${GIT_VERSION}\" $(CFLAGS) $(SRC) -o beagle-tester

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
	$(INSTALL) -m 644 techlab-buzz.out $(DESTDIR)/lib/firmware
	$(INSTALL) -m 644 gamepup-buzz-on-buttons.out $(DESTDIR)/lib/firmware
	$(MAKE) -C images -s install
	#systemctl stop beagle-tester.service || true
	#systemctl daemon-reload || true
	#systemctl enable beagle-tester.service || true

start: install
	systemctl restart beagle-tester.service
