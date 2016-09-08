beagle-tester: beagle-tester.c
	gcc -W -Wall -Wwrite-strings -O3 -o beagle-tester beagle-tester.c

clean:
	rm beagle-tester

install: beagle-tester beagle-tester.service beagle-tester.rules beagle-tester-config.sh
	install -m 755 -d $(DESTDIR)/usr/sbin
	install -m 700 beagle-tester $(DESTDIR)/usr/sbin
	install -m 744 connect_bb_tether $(DESTDIR)/usr/sbin
	install -m 744 beagle-tester-config.sh $(DESTDIR)/usr/sbin
	install -m 755 -d $(DESTDIR)/lib/systemd/system
	install -m 644 beagle-tester.service $(DESTDIR)/lib/systemd/system
	install -m 755 -d $(DESTDIR)/etc/udev/rules.d
	install -m 644 beagle-tester.rules $(DESTDIR)/etc/udev/rules.d
	systemctl stop beagle-tester.service || true
	systemctl daemon-reload || true
	systemctl enable beagle-tester.service || true

start: install
	systemctl restart beagle-tester.service
