lib_LTLIBRARIES = libpractical-sa.la
bin_PROGRAMS = practiparse
noinst_PROGRAMS = practical-sa-ut

libpractical_sa_la_LDFLAGS = -version-info 0:0:0
libpractical_sa_la_SOURCES = practical-sa.cpp practical-errors.cpp scope_tracing.cpp \
			     tokenizer.cpp parser.cpp parser_internal.cpp operators.cpp \
			     ast/ast.cpp ast/cast_op.cpp ast/casts.cpp ast/lookup_context.cpp ast/static_type.cpp \
			     ast/module.cpp ast/function.cpp ast/statement_list.cpp ast/expected_result.cpp ast/statement.cpp \
			     ast/signed_int_value_range.cpp ast/unsigned_int_value_range.cpp ast/mangle.cpp ast/compound_statement.cpp \
			     ast/variable_definition.cpp \
			     ast/expression.cpp ast/expression/base.cpp ast/expression/literal.cpp ast/expression/identifier.cpp \
			     ast/expression/function_call.cpp ast/expression/binary_op.cpp ast/expression/overload_resolver.cpp \
			     ast/expression/compound_expression.cpp \
			     ast/operators/plus.cpp

practical_sa_ut_SOURCES = ut_runner.cpp slice_ut.cpp tokenizer_ut.cpp \
			  tokenizer.cpp exact_int_ut.cpp
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
