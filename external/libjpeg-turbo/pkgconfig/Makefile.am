pcfiles = turbojpeg.pc

all-local: $(pcfiles)

%.pc: %.pc
	cp $< $@

pkgconfigdir= $(libdir)/pkgconfig
pkgconfig_DATA= $(pcfiles)

CLEANFILES= $(pcfiles)

pcinfiles= turbojpeg.pc.in

EXTRA_DIST= $(pcinfiles)

