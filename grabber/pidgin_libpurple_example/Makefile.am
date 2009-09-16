noinst_PROGRAMS = nullclient

nullclient_SOURCES = defines.h nullclient.c nullblist.h nullblist.c nullconv.h nullconv.c nulleventloop.h nulleventloop.c nullsignal.h nullsignal.c
nullclient_DEPENDENCIES =
nullclient_LDFLAGS = -export-dynamic
nullclient_LDADD = \
	$(DBUS_LIBS) \
	$(INTLLIBS) \
	$(GLIB_LIBS) \
	$(LIBXML_LIBS) \
	$(top_builddir)/libpurple/libpurple.la

AM_CPPFLAGS = \
	-DSTANDALONE \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)/purple-$(PURPLE_MAJOR_VERSION)/\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-I$(top_builddir)/libpurple \
	-I$(top_srcdir)/libpurple \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libpurple/protocols/qq \
	$(DEBUG_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(LIBXML_CFLAGS)
