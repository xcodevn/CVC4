SUBDIRS = . arith precedence uf uflra uflia bv arrays aufbv auflia datatypes quantifiers rewriterules lemmas push-pop preprocess tptp unconstrained decision fmf strings
DIST_SUBDIRS = . arith precedence uf uflra uflia bv arrays aufbv auflia datatypes quantifiers rewriterules lemmas push-pop preprocess tptp unconstrained decision fmf strings

# don't override a BINARY imported from a personal.mk
@mk_if@eq ($(BINARY),)
@mk_empty@BINARY = cvc4
end@mk_if@

LOG_COMPILER = @srcdir@/../run_regression
AM_LOG_FLAGS = $(RUN_REGRESSION_ARGS) @top_builddir@/src/main/$(BINARY)$(EXEEXT)

if AUTOMAKE_1_11
# old-style (pre-automake 1.12) test harness
TESTS_ENVIRONMENT = \
	$(TESTS_ENVIRONMENT) $(LOG_COMPILER) \
	$(AM_LOG_FLAGS) $(LOG_FLAGS)
endif

MAKEFLAGS = -k

# These are run for all build profiles.
# If a test shouldn't be run in e.g. competition mode,
# put it below in "TESTS +="

# Regression tests for SMT inputs
SMT_TESTS = \
	distinct.smt \
	flet.smt \
	flet2.smt \
	fuzz_1.smt \
	fuzz_3.smt \
	ineq_basic.smt \
	ineq_slack.smt \
	ite_real_int_type.smt \
	ite_real_valid.smt \
	let.smt \
	let2.smt \
	simplification_bug.smt \
	simplification_bug2.smt \
	simple.smt \
	simple2.smt \
	simple-lra.smt \
	simple-rdl.smt \
	simple-uf.smt \
	constant-rewrite.smt

# Regression tests for SMT2 inputs
SMT2_TESTS = \
	chained-equality.smt2 \
	ite2.smt2 \
	ite3.smt2 \
	ite4.smt2 \
	simple-lra.smt2 \
	simple-rdl.smt2 \
	simple-uf.smt2 \
	simplification_bug4.smt2 \
	parallel-let.smt2 \
	get-value-incremental.smt2 \
	hung13sdk_output1.smt2 \
	hung10_itesdk_output2.smt2 \
	hung10_itesdk_output1.smt2 \
	hung13sdk_output2.smt2

# Regression tests for PL inputs
CVC_TESTS = \
	boolean.cvc \
	boolean-prec.cvc \
	boolean-terms.cvc \
	hole6.cvc \
	ite.cvc \
	let.cvc \
	logops.01.cvc \
	logops.02.cvc \
	logops.03.cvc \
	logops.04.cvc \
	logops.05.cvc \
	simple.cvc \
	smallcnf.cvc \
	test9.cvc \
	test11.cvc \
	uf20-03.cvc \
	wiki.01.cvc \
	wiki.02.cvc \
	wiki.03.cvc \
	wiki.04.cvc \
	wiki.05.cvc \
	wiki.06.cvc \
	wiki.07.cvc \
	wiki.08.cvc \
	wiki.09.cvc \
	wiki.10.cvc \
	wiki.11.cvc \
	wiki.12.cvc \
	wiki.13.cvc \
	wiki.14.cvc \
	wiki.15.cvc \
	wiki.16.cvc \
	wiki.17.cvc \
	wiki.18.cvc \
	wiki.19.cvc \
	wiki.20.cvc \
	wiki.21.cvc \
	simplification_bug3.cvc \
	queries0.cvc \
	print_lambda.cvc

# Regression tests for TPTP inputs
TPTP_TESTS =

# Regression tests derived from bug reports
BUG_TESTS = \
	smt2output.smt2 \
	bug32.cvc \
	bug49.smt \
	bug161.smt \
	bug164.smt \
	bug167.smt \
	bug168.smt \
	bug187.smt2 \
	bug216.smt2 \
	bug217.smt2 \
	bug220.smt2 \
	bug239.smt \
	bug274.cvc \
	bug288.smt \
	bug288b.smt \
	bug288c.smt \
	bug296.smt2 \
	buggy-ite.smt2 \
	bug303.smt2 \
	bug310.cvc \
	bug322.cvc \
	bug322b.cvc \
	bug339.smt2 \
	bug365.smt2 \
	bug382.smt2 \
	bug383.smt2 \
	bug398.smt2 \
	bug411.smt2 \
	bug421.smt2 \
	bug421b.smt2 \
	bug425.cvc \
	bug480.smt2 \
	bug484.smt2 \
	bug486.cvc \
	bug507.smt2 \
	bug512.smt2 \
	bug512.minimized.smt2 \
	bug516.smt2 \
	bug519.smt2 \
	bug520.smt2 \
	bug521.smt2 \
	bug521.minimized.smt2 \
	bug522.smt2

TESTS =	$(SMT_TESTS) $(SMT2_TESTS) $(CVC_TESTS) $(TPTP_TESTS) $(BUG_TESTS)

EXTRA_DIST = $(TESTS) \
	simplification_bug4.smt2.expect \
	bug216.smt2.expect

if CVC4_BUILD_PROFILE_COMPETITION
else
TESTS += \
	error.cvc
endif

# and make sure to distribute it
EXTRA_DIST += \
	subranges.cvc \
	error.cvc

# synonyms for "check" in this directory
.PHONY: regress regress0 test
regress regress0 test: check

# do nothing in this subdir
.PHONY: regress1 regress2 regress3
regress1 regress2 regress3:
