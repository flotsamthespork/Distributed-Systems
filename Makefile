SHELL := /bin/bash

all:
	(cd common && make)
	(cd server && make && cp stringServer ../)
	(cd client && make && cp stringClient ../)

clean:
	(cd common && make clean)
	(cd server && make clean)
	(cd client && make clean)
	rm stringClient stringServer
