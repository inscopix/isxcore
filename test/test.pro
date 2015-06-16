TEMPLATE = subdirs
SUBDIRS =	core \
			TimeGridTest
core.subdir = ..
TimeGridTest.depends = core
