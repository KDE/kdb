# Packaging Information for KDb

We recommend building several binary packages out of the KDb source code.

Splitting KDb into packages:
 * gives users a better choice of which components they have installed;
 * allows users to avoid installing unnecessary dependencies;
 * helps to reduce packaging conflicts for users with non-standard
   package selections.

In this document {MAJOR_VERSION} is 3 for KDb 3.x.y, and so on.


## KDb libraries

The base KDb package offers the following libraries:
 * KDb{MAJOR_VERSION}


## Database and migration drivers

KDb provides database drivers in a form of plugins for a number
of database types or data sources.

* SQLite
  * kdb_sqlitedriver.so - the database driver with the following dependencies:
    * kdb_sqlite_icu.so - SQLite's plugin for unicode support
    * kdb{MAJOR_VERSION}_sqlite3_dump - A minimal command line tool for compacting files

* MySQL
  * kdb_mysqldriver.so - the database driver

* PostgreSQL
  * kdb_postgresqldriver.so - the database driver


Other drivers are work in progress and are not currently distributed.

Plugin `kdb_*driver.so` files typically go to $LIBDIR/plugins/kdb{MAJOR_VERSION}/ directory.
Location of these files means that multiple KDb packages with different MAJOR_VERSIONs
are co-installable. Also header and cmake files are cleanly separated by installing
to subdirectories called KDb{MAJOR_VERSION}.

Please note a special case: KDb 3.0 is binary and source incompatible with KDb >= 3.1.
All other versions are compatible within given MAJOR_VERSION.

We suggest putting each driver in a separate package, and that installation of
these packages be optional. Each driver's package may then depend on the
corresponding lower-level, native client libraries for performing connectivity.
For example, it's libmysqlclient for the MySQL driver and libpq for PostgreSQL driver.


## Versions of client libraries

### For SQLite

KDb's SQLite driver uses the sqlite3 library. Exact minimal version of the
sqlite3 package is defined in the source code and can be found in the
following line of the kdb/src/drivers/CMakeLists.txt file:

set(SQLITE_MIN_VERSION x.y.z)

The recommended version of SQLite package is defined in the source code and can
be found in the following line of the kdb/src/drivers/CMakeLists.txt file:

set(SQLITE_RECOMMENDED_VERSION x.y.z)

### For MySQL

KDb's MySQL driver uses MySQL client library version 5.x.

### For PostgreSQL

KDb's PostgreSQL driver uses pq client library version 9.x.


## More information

KDb wiki page provides useful and most up-to-date information: https://community.kde.org/KDb/Build.
