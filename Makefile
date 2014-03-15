SHELL := /bin/bash

all:
	(cd common && make)
	(cd binder_src && make && cp binder ../)
	(cd server_src && make && cp server ../)
	(cd client_src && make && cp client ../)

clean:
	(cd common && make clean)
	(cd server_src && make clean)
	(cd client_src && make clean)
	(cd binder_src && make clean)
	rm client server binder
