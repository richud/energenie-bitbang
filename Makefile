LDFLAGS += -lftdi1 -lusb-1.0
INSTALL ?= install
PROG = energenie
OUTPUT ?= /usr/bin
STRIP ?= strip

all: $(PROG) 

clean: 
	rm -f $(PROG)

install: $(PROG) strip
	$(INSTALL) -m 755 "$(PROG)" "$(DESTDIR)$(OUTPUT)"

$(PROG): $(PROG).c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) 

strip:
	$(STRIP) $(PROG)

