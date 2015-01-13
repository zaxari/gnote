
include $(builddir)/../addins.mk

@INTLTOOL_DESKTOP_RULE@

desktop_in_files = repo.desktop.in
desktop_files    = $(desktop_in_files:.desktop.in=.desktop)

addinsdir = $(ADDINSDIR)
addins_LTLIBRARIES = repo.la
addins_DATA = $(desktop_files)

repo_la_SOURCES = repo.hpp repo.cpp \
		  repopreferencesfactory.hpp repopreferences.hpp \
		  repopreferences.cpp gitcli.hpp gitcli.cpp $(NULL)

EXTRA_DIST = $(desktop_in_files)
DISTCLEANFILES = $(desktop_files)
AM_CPPFLAGS = @libgit2_CFLAGS@
AM_CXXFLAGS = @libgit2_CFLAGS@
AM_CFLAGS = @libgit2_CFLAGS@


repo_la_LIBADD = @libgit2_LIBS@ -lgit2
