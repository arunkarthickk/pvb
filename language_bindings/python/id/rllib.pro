######################################################################
# generated by pvdevelop at: Mi Nov 8 11:58:45 2006
######################################################################

TEMPLATE = lib
CONFIG   = warn_on release qt
QT      += sql

# Input
HEADERS      +=
SOURCES      += ../../language_binding_rllib_wrap_python.cxx
INCLUDEPATH  += ${PYTHON_INCLUDE}
INCLUDEPATH  += ../../../pvserver

DEFINES += USE_INETD
TARGET = _rllib