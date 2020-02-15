lib_LTLIBRARIES = libpractical-sa.la
bin_PROGRAMS = practiparse
noinst_PROGRAMS = practical-sa-ut

libpractical_sa_la_LDFLAGS = -version-info 0:0:0
libpractical_sa_la_SOURCES = practical-sa.cpp tokenizer.cpp parser.cpp parser_internal.cpp scope_tracing.cpp lookup_context.cpp ast.cpp ast_nodes.cpp \
			     practical-errors.cpp operators.cpp expression.cpp \
			     full_range_int.cpp
# XXX temporarily disabled: casts.cpp dummy_codegen_impl.cpp operators.cpp codegen.cpp codegen_operators.cpp

practical_sa_ut_SOURCES = ut_runner.cpp slice_ut.cpp tokenizer_ut.cpp tokenizer.cpp exact_int_ut.cpp full_range_int.cpp full_range_int_ut.cpp
# We need automake to compile cpp files for the UTs distinctly than for the library. We do this by adding a useless compile flag
# that applies only to the UTs executable. Otherwise we can't use the same CPP files for both library and executable
practical_sa_ut_CPPFLAGS = -I$(top_srcdir)/include
practical_sa_ut_LDADD = @CPPUNIT_LIBS@
practical_sa_ut_CFLAGS = @CPPUNIT_CFLAGS@ $(AM_CFLAGS)

practiparse_SOURCES = practiparse.cpp
practiparse_LDADD = libpractical-sa.la
practiparse_LDFLAGS = -static
practiparse_DEPENDENCIES = libpractical-sa.la

ut: practical-sa-ut$(EXEEXT)
	TOP_DIR="$(top_srcdir)" $(builddir)/practical-sa-ut

.PHONY: ut
