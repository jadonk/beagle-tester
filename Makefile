prefix := /usr

all: beagle-tester images

beagle-tester: beagle-tester.c
	gcc -W -Wall -Wwrite-strings -O3 -o beagle-tester beagle-tester.c

images:
	make -C images

clean:
	rm beagle-tester

install:
	install -m 755 -d $(DESTDIR)$(prefix)/sbin
	install -m 700 beagle-tester $(DESTDIR)$(prefix)/sbin
	install -m 744 connect_bb_tether $(DESTDIR)$(prefix)/sbin
	install -m 744 beagle-tester-open.sh $(DESTDIR)$(prefix)/sbin
	install -m 744 beagle-tester-close.sh $(DESTDIR)$(prefix)/sbin
	install -m 755 -d $(DESTDIR)/lib/systemd/system
	install -m 644 beagle-tester.service $(DESTDIR)/lib/systemd/system
	install -m 755 -d $(DESTDIR)/etc/udev/rules.d
	install -m 644 beagle-tester.rules $(DESTDIR)/etc/udev/rules.d
	make -C images -s install
	systemctl stop beagle-tester.service || true
	systemctl daemon-reload || true
	systemctl enable beagle-tester.service || true

start: install
	systemctl restart beagle-tester.service
