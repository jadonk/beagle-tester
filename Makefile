beagle-tester: beagle-tester.c
	gcc -O3 -o beagle-tester beagle-tester.c

clean:
	rm beagle-tester

install: beagle-tester beagle-tester.service beagle-tester.rules
	install -m 700 beagle-tester /usr/sbin
	install -m 644 beagle-tester.service /usr/lib
	install -m 644 beagle-tester.rules /etc/udev/rules.d
	systemctl stop beagle-tester.service
	systemctl daemon-reload
	systemctl enable beagle-tester.service
