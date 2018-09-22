
lib_LTLIBRARIES = libpractical-sa.la
noinst_PROGRAMS = practical-sa-ut

libpractical_sa_la_LDFLAGS = -version-info 0:0:0
libpractical_sa_la_SOURCES = tokenizer.cpp

practical_sa_ut_SOURCES = ut_runner.cpp slice_ut.cpp
practical_sa_ut_LDADD = @CPPUNIT_LIBS@
practical_sa_ut_CFLAGS = @CPPUNIT_CFLAGS@ $(AM_CFLAGS)
