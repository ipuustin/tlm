## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tlm-client addseat

tlm_client_SOURCES = tlm-client.c
tlm_client_CFLAGS = \
    -I$(top_builddir)/src \
    -I$(top_srcdir)/src \
    $(TLM_INCLUDES) \
    $(TLM_CFLAGS) \
    -DTLM_BIN_DIR='"$(bindir)"' \
    $(DEPS_CFLAGS)

tlm_client_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/src/common/libtlm-common.la \
	$(top_builddir)/src/common/dbus/libtlm-dbus-glue.la \
	$(TLM_LIBS)


addseat_SOURCES = addseat.c
addseat_CFLAGS = \
    -I$(top_builddir)/src \
    -I$(top_srcdir)/src \
    $(TLM_INCLUDES) \
    $(TLM_CFLAGS) \
    -DTLM_BIN_DIR='"$(bindir)"' \
    $(DEPS_CFLAGS)

addseat_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/src/common/libtlm-common.la \
	$(TLM_LIBS)


CLEANFILES = *.gcno *.gcda
