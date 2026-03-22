SUBDIRS:=src

.PHONY: all clean $(SUBDIRS)

CFLAGS:=	-Wall -O2 -MMD -MP
export CFLAGS

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
install:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
uninstall:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
