TEMPLATE	= lib

include( $(KEXI)/kexidb/common.pro )

# needed to export library classes:
DEFINES += MAKE_KEXI_DB_LIB

TARGET		= kexidb$$KDEBUG

system( moc connection.h -o moc/connection.moc )

#system( moc kexidb.h -o moc/kexidb.moc )
#system( moc kexidbinterfacemanager.h -o moc/kexidbinterfacemanager.moc )
#system( moc kexidbrecordset.h -o moc/kexidbrecordset.moc )
#system( moc kexidbwatcher.h -o moc/kexidbwatcher.moc )
#system( moc kexidbrecordset.h -o moc/kexidbrecordset.moc )

SOURCES = \
object.cpp \
drivermanager.cpp \
driver.cpp \
connectiondata.cpp \
connection.cpp \
field.cpp \
schemadata.cpp \
tableschema.cpp \
queryschema.cpp \
transaction.cpp \
indexschema.cpp \
cursor.cpp \
fieldlist.cpp \
global.cpp \
\
parser/parser.cpp \
parser/sqlparser.cpp \
parser/sqlscanner.cpp

#HEADERS =
