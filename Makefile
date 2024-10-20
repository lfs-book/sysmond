objects = sysmond.o args.o udp.o daemonize.o signal.o data.o temps.o utils.o

sysmond: $(objects)
	gcc -o sysmond $(CFLAGS) -s $(objects)

$(objects): %.o: %.c
	gcc -c $(CFLAGS) -O2 $< -o $@

sysmond.o:   sysmond.c   sysmond.h 
args.o:      args.c      sysmond.h args.h
udp.o:       udp.c       sysmond.h udp.h utils.h
daemonize.o: daemonize.c sysmond.h
signal.o:    signal.c    sysmond.h
data.o:      data.c      sysmond.h
temps.o:     temps.c     sysmond.h
utils.o:     utils.c     utils.h

install:
	cp -v sysmond /usr/sbin

clean:
	rm -f $(objects) sysmond
	
.PHONY: clean
