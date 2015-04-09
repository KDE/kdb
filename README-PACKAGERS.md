# Packaging Information for KDb

We recommend building several binary packages out of the KDb source code.

Splitting KDb into packages:
 * gives users a better choice of which components they have installed;
 * allows users to avoid installing unnecessary dependencies;
 * helps to reduce packaging conflicts for users with non-standard
   package selections.


## KDb libraries

KDb offers the following libraries:
 * kdb


## Database and migration drivers

KDb provides database drivers in a form of plugins for a number
of database types or data sources.

* SQLite
  * kdb_sqlite.so - the database driver
  * kdb_sqlite.desktop
  * kdb_sqlite_icu.so - SQLite's plugin for unicode support
  * kdb_sqlite3_dump - A minimal command line tool for compacting files

* MySQL
  * kdb_mysql.so - the database driver
  * kdb_mysql.desktop

* PostgreSQL
  * kdb_postgresql.so - the database driver
  * kdb_postgresql.desktop


Other drivers are work in progress and are not currently distributed.

Plugin .so and .desktop service files typically go
to $PREFIX/lib/plugins/kdb/ directory.

We suggest putting each driver in a separate package, and that installation of
these packages be optional. Each driver's package may then depend on the
corresponding lower-level, native client libraries for performing connectivity.
For example, it's libmysqlclient for the MySQL driver and libpq for PostgreSQL.


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

KDb wiki page provides useful and most up-to-date information: http://community.kde.org/KDb/Build.
