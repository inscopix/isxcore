TEMPLATE = subdirs
SUBDIRS =	core \
			TimeTest \
			TimeGridTest
core.subdir = ..
TimeGridTest.depends = core
TimeTest.depends = core
