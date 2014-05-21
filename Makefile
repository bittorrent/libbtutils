#
# Host detection
#

include host.mk
ifeq ($(HOST),)
$(error "Can't identify host")
endif

#
# Target specification
#

TARGET_ANDROID = android

$(info Host $(HOST))
ifeq ($(TARGET),)
$(info Using default target of $(HOST) (same as host))
TARGET = $(HOST)
else
ifneq ($(TARGET),$(HOST_LINUX))
ifneq ($(TARGET),$(HOST_MAC))
ifneq ($(TARGET),$(HOST_CYGWIN))
ifneq ($(TARGET),$(HOST_FREEBSD))
ifneq ($(TARGET),$(TARGET_ANDROID))
$(error Invalid target: '$(TARGET)'. Valid targets: $(HOST_LINUX), $(HOST_MAC), $(HOST_CYGWIN), $(HOST_FREEBSD), $(TARGET_ANDROID) (e.g. make TARGET=$(HOST)))
endif
endif
endif
endif
endif
$(info Target $(TARGET))
endif

#
# Platform Setup
#

export TARGET
include platform.mk
ifeq ($(PLATFORM_FLAGS),)
$(error "Can't initialize for target $(TARGET)")
endif

#
# Build setup
#

export CONFIG
export CHARSET
export OPTIMIZE
include buildconfig.mk
ifeq ($(CONFIG),)
$(error "Can't initialize configuration (CONFIG)")
endif
ifeq ($(CHARSET),)
$(error "Can't initialize configuration (CHARSET)")
endif
ifeq ($(OPTIMIZE),)
$(error "Can't initialize configuration (OPTIMIZE)")
endif

#
# Compile and linkage setup
#

# Show system on which we build, gcc version and machine name. Will be useful when
# looking at log output of failed compilation.
$(info Building on $(shell uname -a))
$(info gcc version: $(shell $(CC) -dumpversion))
$(info gcc machine: $(shell $(CC) -dumpmachine))
$(info )

# Compile flags

# -c - compile/assemble source files, but do not link
# -MD - output dependencies to a file located with the object file
# -g - provide debugging information in OS's native format (for gdb)
# -pipe - use pipes instead of temporary files for comm between compilation stages
# -Wall - enable (almost) all warnings
# -Werror - make all warnings into errors
# -O - optimization level
UTILS_COMMON_FLAGS = \
	-c \
	-MD \
	-g \
	-pipe \
	-Wall \
	-O$(OPTIMIZE)

# -std - specify the language standard
UTILS_CXXONLY_FLAGS = \
	-std=c++11

ifeq ($(CHARSET),$(CHARSET_UNICODE))
UTILS_COMMON_FLAGS += -D_UNICODE
endif

ifeq ($(CONFIG),$(CONFIG_DEBUG))
UTILS_COMMON_FLAGS += -D_DEBUG
endif

OUTDIR_PREFIX = obj-
# Keep same order of parts as in ut_ce
OUTDIR = $(OUTDIR_PREFIX)$(CHARSET)-$(CONFIG)-O$(OPTIMIZE)

# Link flags

LD_SYSTEM_SHLIBFLAGS = -lpthread -lm

ifeq ($(TARGET),$(HOST_LINUX))
# -lrt is for clock_gettime() - see its man page
LD_SYSTEM_SHLIBFLAGS += -lrt
endif

# Include all added compile flags in CFLAGS and CXXFLAGS
UTILS_CFLAGS = $(UTILS_COMMON_FLAGS) $(PLATFORM_FLAGS)
UTILS_CXXFLAGS = $(UTILS_COMMON_FLAGS) $(PLATFORM_FLAGS) $(UTILS_CXXONLY_FLAGS)
# Android doesn't like CFLAGS/CXXFLAGS modified in this file.
# It may be useful to avoid setting LOCAL_ flags here - see later.
ifeq ($(TARGET),$(TARGET_ANDROID))
LOCAL_CFLAGS += $(UTILS_CFLAGS)
LOCAL_CXXFLAGS += $(UTILS_CXXFLAGS)
endif

#
# Library sources and products
#

SRC_DIR = src
LIBSRCS = $(sort $(addprefix $(SRC_DIR)/, \
	DecodeEncodedString.cpp \
	RefBase.cpp \
	bencoding.cpp \
	bencparser.cpp \
	bitfield.cpp \
	bloom_filter.cpp \
	get_microseconds.cpp \
	inet_ntop.cpp \
	interlock.cpp \
	snprintf.cpp \
	sockaddr.cpp \
))

LIBBASENAME = ututils
LIBNAME = lib$(LIBBASENAME).so
LIBOBJS = $(patsubst %.cpp, $(OUTDIR)/%.o, $(LIBSRCS))
LIBOBJDIR = $(OUTDIR)/$(SRC_DIR)
LIBDESTDIR = $(OUTDIR)
OBJDESTLIB = $(LIBDESTDIR)/$(LIBNAME)
UNSTRIPPEDLIBDESTDIR = $(LIBOBJDIR)
OBJDESTUNSTRIPPEDLIB = $(UNSTRIPPEDLIBDESTDIR)/$(LIBNAME)

LIBRARY_CXXFLAGS = -fPIC
LIBRARY_BUILDFLAGS = -shared

#
# Unit tests sources and products
#

GTEST_VERSION = 1.6.0
GOOGLE_TEST_DIR = vendor/gtest-$(GTEST_VERSION)/src
GOOGLE_MOCK_DIR = vendor/gmock-$(GTEST_VERSION)/src
SRCS_GOOGLE_TEST = $(addprefix $(GOOGLE_TEST_DIR)/, \
	gtest-all.cc \
)
SRCS_GOOGLE_MOCK = $(addprefix $(GOOGLE_MOCK_DIR)/, \
	gmock-all.cc \
	gmock_main.cc \
)
OBJS_GOOGLE_TEST = $(patsubst %.cc, $(OUTDIR)/%.o, $(SRCS_GOOGLE_TEST))
OBJS_GOOGLE_MOCK = $(patsubst %.cc, $(OUTDIR)/%.o, $(SRCS_GOOGLE_MOCK))
OUTPUT_GOOGLE_TEST_DIR = $(OUTDIR)/$(GOOGLE_TEST_DIR)
OUTPUT_GOOGLE_MOCK_DIR = $(OUTDIR)/$(GOOGLE_MOCK_DIR)

UNITTESTS_DIR = unittests
LIBUNITTESTOBJDIR = $(OUTDIR)/$(UNITTESTS_DIR)
SRCS_TESTS = $(addprefix $(UNITTESTS_DIR)/, \
	TestBencEntity.cpp \
	TestBencoding.cpp \
	TestBitField.cpp \
	TestBloomFilter.cpp \
	TestGetMicroseconds.cpp \
	TestSha1Hash.cpp \
	TestSockAddr.cpp \
)
OBJS_TESTS = $(patsubst %.cpp, $(OUTDIR)/%.o, $(SRCS_TESTS))
UNITTESTS_EXE_NAME = unit_tests
UT_EXE_DEST = $(LIBUNITTESTOBJDIR)/$(UNITTESTS_EXE_NAME)

SRCS_UNITTESTS = \
	$(SRCS_GOOGLE_TEST) \
	$(SRCS_TESTS)

OBJS_UNITTESTS = $(OBJS_TESTS) $(OBJS_GOOGLE_TEST) $(OBJS_GOOGLE_MOCK)

INCLUDE_UNITTESTS = \
	-I$(SRC_DIR)/ \
	-Ivendor/gtest-$(GTEST_VERSION)/include \
	-Ivendor/gtest-$(GTEST_VERSION) \
	-Ivendor/gmock-$(GTEST_VERSION)/include \
	-Ivendor/gmock-$(GTEST_VERSION)

LD_COMPONENT_SHLIBFLAGS = -L$(UNSTRIPPEDLIBDESTDIR) -l$(LIBBASENAME)
LD_SHLIB_FLAGS = $(LD_COMPONENT_SHLIBFLAGS) $(LD_SYSTEM_SHLIBFLAGS)

# Valgrind testing

VALGRIND_LOG = $(OUTDIR)/$(UNITTESTS_EXE_NAME).valgrind.log
VALGRIND_ARGS = \
	--tool=memcheck \
	--leak-check=yes \
	--log-file=$(VALGRIND_LOG) \
	--error-exitcode=1

#
# Rules
#

# Build/test rules

.phony: all test product vgtest

all: $(OBJDESTLIB) $(UT_EXE_DEST)

product: $(OBJDESTLIB)

test: $(UT_EXE_DEST)
	env LD_LIBRARY_PATH=$(UNSTRIPPEDLIBDESTDIR) $<

vgtest: $(UT_EXE_DEST)
	env LD_LIBRARY_PATH=$(UNSTRIPPEDLIBDESTDIR) valgrind $(VALGRIND_ARGS) $<
	@echo "Checking for 'Invalid free'"
	@grep "Invalid free" $(VALGRIND_LOG) ; SEARCH_RESULT=$$? ; if [ $$SEARCH_RESULT -eq 1 ] ; then exit 0 ; else exit 1 ; fi
	@echo "Checking for 'Invalid write of size'"
	@grep "Invalid write of size" $(VALGRIND_LOG) ; SEARCH_RESULT=$$? ; if [ $$SEARCH_RESULT -eq 1 ] ; then exit 0 ; else exit 1 ; fi
	@echo "Checking for 'Process terminating with default action of signal'"
	@grep "Process terminating with default action of signal" $(VALGRIND_LOG) ; SEARCH_RESULT=$$? ; if [ $$SEARCH_RESULT -eq 1 ] ; then exit 0 ; else exit 1 ; fi
	@echo "Checking for 'Bad permissions for mapped region at address'"
	@grep "Bad permissions for mapped region at address" $(VALGRIND_LOG) ; SEARCH_RESULT=$$? ; if [ $$SEARCH_RESULT -eq 1 ] ; then exit 0 ; else exit 1 ; fi
	@echo "No error strings found in $(VALGRIND_LOG)"

$(OBJDESTLIB): $(OBJDESTUNSTRIPPEDLIB)
	strip -S -o $@ $<

$(OBJDESTUNSTRIPPEDLIB): $(LIBOBJS) $(filter-out $(wildcard $(UNSTRIPPEDLIBDESTDIR)), $(UNSTRIPPEDLIBDESTDIR))
	$(CXX) -o $@ $(LIBRARY_BUILDFLAGS) $(LIBOBJS)

$(UT_EXE_DEST): $(OBJDESTUNSTRIPPEDLIB) $(OBJS_UNITTESTS) $(filter-out $(wildcard $(LIBUNITTESTOBJDIR)), $(LIBUNITTESTOBJDIR))
	$(CXX) -o $@ $(OBJS_UNITTESTS) $(LD_SHLIB_FLAGS)

# Output directory creation rules

# $(filter-out $(wildcard $(directorymacro)), $(directorymacro)) establishes
# a dependency on a directory that doesn't already exist, so that if the
# directory exists, the associated mkdir command won't be executed,
# which would prevent a clean no-op when nothing really needs doing.

$(OUTDIR):
	mkdir -p $@

ifneq ($(UNSTRIPPEDLIBDESTDIR),$(LIBOBJDIR))
$(UNSTRIPPEDLIBDESTDIR): $(filter-out $(wildcard $(OUTDIR)), $(OUTDIR))
	mkdir -p $@

endif

$(LIBOBJDIR): $(filter-out $(wildcard $(OUTDIR)), $(OUTDIR))
	mkdir -p $@

$(LIBUNITTESTOBJDIR): $(filter-out $(wildcard $(OUTDIR)), $(OUTDIR))
	mkdir -p $@

$(OUTPUT_GOOGLE_TEST_DIR): $(filter-out $(wildcard $(OUTDIR)), $(OUTDIR))
	mkdir -p $@

$(OUTPUT_GOOGLE_MOCK_DIR): $(filter-out $(wildcard $(OUTDIR)), $(OUTDIR))
	mkdir -p $@

# Implicit rules

$(LIBOBJS): $(LIBOBJDIR)/%.o: $(SRC_DIR)/%.cpp $(filter-out $(wildcard $(LIBOBJDIR)), $(LIBOBJDIR))
	$(CXX) $(UTILS_CXXFLAGS) $(LIBRARY_CXXFLAGS) -o $@ $<

$(OBJS_TESTS): $(LIBUNITTESTOBJDIR)/%.o: $(UNITTESTS_DIR)/%.cpp $(filter-out $(wildcard $(LIBUNITTESTOBJDIR)), $(LIBUNITTESTOBJDIR))
	$(CXX) $(UTILS_CXXFLAGS) $(INCLUDE_UNITTESTS) -o $@ $< -DGTEST_USE_OWN_TR1_TUPLE=1

$(OBJS_GOOGLE_TEST): $(OUTPUT_GOOGLE_TEST_DIR)/%.o: $(GOOGLE_TEST_DIR)/%.cc $(filter-out $(wildcard $(OUTPUT_GOOGLE_TEST_DIR)), $(OUTPUT_GOOGLE_TEST_DIR))
	$(CXX) $(UTILS_CXXFLAGS) $(INCLUDE_UNITTESTS) -o $@ $< -DGTEST_USE_OWN_TR1_TUPLE=1

$(OBJS_GOOGLE_MOCK): $(OUTPUT_GOOGLE_MOCK_DIR)/%.o: $(GOOGLE_MOCK_DIR)/%.cc $(filter-out $(wildcard $(OUTPUT_GOOGLE_MOCK_DIR)), $(OUTPUT_GOOGLE_MOCK_DIR))
	$(CXX) $(UTILS_CXXFLAGS) $(INCLUDE_UNITTESTS) -o $@ $< -DGTEST_USE_OWN_TR1_TUPLE=1

# Clean rules

.phony: clean cleanall

clean:
	rm -rf $(OUTDIR)

cleanall:
	rm -rf $(OUTDIR_PREFIX)*
