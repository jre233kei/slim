
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/test/tap-driver.sh

TESTS = \
	testsuite/integer/check.sh \
	testsuite/set/check.sh \
	testsuite/statespace/check.sh

check_DATA = \
	testsuite/integer/integer_set.il \
	testsuite/set/set.il \
	testsuite/statespace/statespace.il

CLEANFILES = $(check_DATA)

LMNC = $(LMNTAL_HOME)/bin/lmntal
LMNCFLAGS = --hl --slimcode -O3

%.il: %.lmn
	$(LMNC) $(LMNCFLAGS) $< >$@
