PREFIX = /etc/mevent

install:
	sudo install -d $(PREFIX)
	sudo install -m 0644 server.hdf $(PREFIX)/
	sudo install -m 0644 client.hdf $(PREFIX)/

uninstall:
	sudo rm -f $(PREFIX)/server.hdf
	sudo rm -f $(PREFIX)/client.hdf

clean:
