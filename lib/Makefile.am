lib_LTLIBRARIES = libpractical-sa.la
noinst_PROGRAMS = practical-sa-ut practiparse

libpractical_sa_la_LDFLAGS = -version-info 0:0:0
libpractical_sa_la_SOURCES = tokenizer.cpp parser.cpp scope_tracing.cpp semantic_analyzer.cpp

practical_sa_ut_SOURCES = ut_runner.cpp slice_ut.cpp tokenizer_ut.cpp tokenizer.cpp
# We need automake to compile cpp files for the UTs distinctly than for the library. We do this by adding a useless compile flag
# that applies only to the UTs executable. Otherwise we can't use the same CPP files for both library and UT
practical_sa_ut_CPPFLAGS = -I.
practical_sa_ut_LDADD = @CPPUNIT_LIBS@
practical_sa_ut_CFLAGS = @CPPUNIT_CFLAGS@ $(AM_CFLAGS)

practiparse_SOURCES = practiparse.cpp
practiparse_LDADD = libpractical-sa.la
practiparse_LDFLAGS = -static
practiparse_DEPENDENCIES = libpractical-sa.la

ut: practical-sa-ut$(EXEEXT)
	TOP_DIR="$(top_srcdir)" $(builddir)/practical-sa-ut

.PHONY: ut
