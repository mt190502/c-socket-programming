SUBDIRS = single-connection-tcp-echo-server single-connection-unix-socket-echo-server udp-echo-server

all: compile move 

compile:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

move:
	mkdir -p bin
	@for dir in $(SUBDIRS); do \
		mv $$dir/server bin/$$dir-server; \
		mv $$dir/client bin/$$dir-client; \
	done

clean:
	rm -rf bin


.PHONY: all move clean