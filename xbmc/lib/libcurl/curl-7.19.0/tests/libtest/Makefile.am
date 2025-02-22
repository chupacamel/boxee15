#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# $Id: Makefile.am,v 1.74 2008-08-18 09:58:11 yangtse Exp $
###########################################################################
AUTOMAKE_OPTIONS = foreign nostdinc

# Specify our include paths here, and do it relative to $(top_srcdir) and
# $(top_builddir), to ensure that these paths which belong to the library
# being currently built and tested are searched before the library which
# might possibly already be installed in the system.
#
# $(top_builddir)/include is for libcurl's generated curl/curlbuild.h file
# $(top_srcdir)/include is for libcurl's external include files
# $(top_builddir)/lib is for libcurl's generated lib/config.h file
# $(top_srcdir)/lib is for libcurl's lib/setup.h and other "borrowed" files

INCLUDES = -I$(top_builddir)/include \
           -I$(top_srcdir)/include   \
           -I$(top_builddir)/lib     \
           -I$(top_srcdir)/lib 

LIBDIR = $(top_builddir)/lib

EXTRA_DIST = test75.pl test307.pl test610.pl test613.pl test1013.pl test1022.pl

# files used only in some libcurl test programs
TESTUTIL = testutil.c testutil.h

# these files are used in every single test program below
SUPPORTFILES = first.c test.h

# These are all libcurl test programs
noinst_PROGRAMS = lib500 lib501 lib502 lib503 lib504 lib505 lib506	\
  lib507 lib508 lib510 lib511 lib512 lib513 lib514 lib515 lib516	\
  lib517 lib518 lib519 lib520 lib521 lib523 lib524 lib525 lib526 lib527	\
  lib529 lib530 lib532 lib533 lib536 lib537 lib540 lib541 lib542 lib543 \
  lib544 lib545 lib547 lib548 lib549 lib552 lib553 lib554 lib555 lib556 \
  lib539 lib557

# Dependencies (may need to be overriden)
LDADD = $(LIBDIR)/libcurl.la
DEPENDENCIES = $(LIBDIR)/libcurl.la


lib500_SOURCES = lib500.c $(SUPPORTFILES)

lib501_SOURCES = lib501.c $(SUPPORTFILES)

lib502_SOURCES = lib502.c $(SUPPORTFILES) $(TESTUTIL)

lib503_SOURCES = lib503.c $(SUPPORTFILES) $(TESTUTIL)

lib504_SOURCES = lib504.c $(SUPPORTFILES) $(TESTUTIL)

lib505_SOURCES = lib505.c $(SUPPORTFILES)

lib506_SOURCES = lib506.c $(SUPPORTFILES)

lib507_SOURCES = lib507.c $(SUPPORTFILES) $(TESTUTIL)

lib508_SOURCES = lib508.c $(SUPPORTFILES)

lib510_SOURCES = lib510.c $(SUPPORTFILES)

lib511_SOURCES = lib511.c $(SUPPORTFILES)

lib512_SOURCES = lib512.c $(SUPPORTFILES)

lib513_SOURCES = lib513.c $(SUPPORTFILES)

lib514_SOURCES = lib514.c $(SUPPORTFILES)

lib515_SOURCES = lib515.c $(SUPPORTFILES)

lib516_SOURCES = lib516.c $(SUPPORTFILES)

lib517_SOURCES = lib517.c $(SUPPORTFILES)

lib518_SOURCES = lib518.c $(SUPPORTFILES)

lib519_SOURCES = lib519.c $(SUPPORTFILES)

lib520_SOURCES = lib520.c $(SUPPORTFILES)

lib521_SOURCES = lib521.c $(SUPPORTFILES)

lib523_SOURCES = lib523.c $(SUPPORTFILES)

lib524_SOURCES = lib524.c $(SUPPORTFILES)

lib525_SOURCES = lib525.c $(SUPPORTFILES) $(TESTUTIL)

lib526_SOURCES = lib526.c $(SUPPORTFILES) $(TESTUTIL)
lib526_CFLAGS = -DLIB526

lib527_SOURCES = lib526.c $(SUPPORTFILES) $(TESTUTIL)
lib527_CFLAGS = -DLIB527

lib529_SOURCES = lib525.c $(SUPPORTFILES) $(TESTUTIL)
lib529_CFLAGS = -DLIB529

lib530_SOURCES = lib530.c $(SUPPORTFILES) $(TESTUTIL)
lib530_CFLAGS = -DLIB530

lib532_SOURCES = lib526.c $(SUPPORTFILES) $(TESTUTIL)
lib532_CFLAGS = -DLIB532

lib533_SOURCES = lib533.c $(SUPPORTFILES) $(TESTUTIL)

lib536_SOURCES = lib536.c $(SUPPORTFILES) $(TESTUTIL)

lib537_SOURCES = lib537.c $(SUPPORTFILES)

lib539_SOURCES = lib539.c $(SUPPORTFILES)

lib540_SOURCES = lib540.c $(SUPPORTFILES)

lib541_SOURCES = lib541.c $(SUPPORTFILES)

lib542_SOURCES = lib542.c $(SUPPORTFILES)

lib543_SOURCES = lib543.c $(SUPPORTFILES)

lib544_SOURCES = lib544.c $(SUPPORTFILES)

lib545_SOURCES = lib544.c $(SUPPORTFILES)
lib545_CFLAGS = -DLIB545

lib547_SOURCES = lib547.c $(SUPPORTFILES)

lib548_SOURCES = lib547.c $(SUPPORTFILES)
lib548_CFLAGS = -DLIB548

lib549_SOURCES = lib549.c $(SUPPORTFILES)

lib555_SOURCES = lib555.c $(SUPPORTFILES) $(TESTUTIL)

lib552_SOURCES = lib552.c $(SUPPORTFILES)

lib553_SOURCES = lib553.c $(SUPPORTFILES)

lib554_SOURCES = lib554.c $(SUPPORTFILES)

lib556_SOURCES = lib556.c $(SUPPORTFILES)

lib557_SOURCES = lib557.c $(SUPPORTFILES)
