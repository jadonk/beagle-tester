beagle-tester: beagle-tester.c
	gcc -W -Wall -Wwrite-strings -O3 -o beagle-tester beagle-tester.c

clean:
	rm beagle-tester

install: beagle-tester beagle-tester.service beagle-tester.rules beagle-tester-config.sh
	install -m 700 beagle-tester /usr/sbin
	install -m 744 beagle-tester-config.sh /usr/sbin
	install -m 644 beagle-tester.service /usr/lib
	install -m 644 beagle-tester.rules /etc/udev/rules.d
	systemctl stop beagle-tester.service
	systemctl daemon-reload
	systemctl enable beagle-tester.service

start: install
	systemctl restart beagle-tester.service
