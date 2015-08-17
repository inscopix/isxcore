TEMPLATE = subdirs
SUBDIRS =   core\
            TimeTest\
            TimeGridTest\
            PointTest
core.subdir = ..
TimeGridTest.depends = core
TimeTest.depends = core
PointTest.depends = core
