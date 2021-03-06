#!/bin/bash
################################################################################
# sql_keywords.sh
#
# Generate sets of driver-specific keywords.
# This program generates files that can be used as part of KDb drivers
# that list keywords specific to that driver, i.e. words that have to be
# escaped if they are to be used as identifiers in the database.
#
# It extracts keywords from the lexer of the DB sources, deletes keywords that
# are already going to be escaped because they are part of KDb's SQL dialect,
# and writes the resulting keywords to a "char *keywords[]" construct in a .cpp 
# file that can then be used in the driver.
#
# To use:
# Put the DB source tarballs/sources (e.g. mysql-4.1.7.tar.gz, 
# postgresql-base-7.4.6.tar.gz, SQLite's tokenize.c) in the current directory
# then run.
#
# sed, awk, grep have been used without much thought -
# CHECK THE OUTPUT BEFORE INCLUDING IT IN A DRIVER!
#
# 2004 Martin Ellis <martin.ellis@kdemail.net>
# 2011 Jarosław Staniek <staniek@kde.org> - adopted for KDb

set -e
progname="sql_keywords.sh"

################################################################################
# C++ file generator
# params : array   - scoped datatype and name of the array to generate
#          include - a file to include (or "" if none)
#          inFile  - file containing raw keywords
#          outfile - file to write 
header () {
  local array="$1"
  local include="$2"
  local inFile="$3"
  local outFile="$4"
  echo "Writing keywords in $inFile to $outFile"
  cat <<EOF1 > "$outFile";
/* This file is part of the KDE project
   Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

   This file has been automatically generated from
   tools/$progname and $inFile.

   Please edit the $progname, not this file!

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
EOF1
  if [ "$include" != "" ] ; then
    echo "#include <$include>" >> "$outFile"
  fi
  cat <<EOF2 >> "$outFile";

  ${array}[] = {
EOF2
}

body() {
  local inFile="$1"
  local outFile="$2"
  awk '/^[a-zA-Z_0-9]*/ { print "\t\t\""$$1"\","; } ' "$inFile" >> "$outFile"
}

footer() {
  local outFile="$1"
  cat <<EOF >> "$outFile";
		0
  };
}
EOF

}

################################################################################
# Keyword comparison functions
# Globals: keywords

# readKeywords
# params: filename - file of keywords to read
# sets:   keywords - array of keywords in the file
readKeywords () {
  local filename="$1" 
  local kdbSQL="$2"
  i=0
  while read keyword ; do
    keywords[$i]="$keyword"
    (( i++ ))
  done < "$filename"
}

# compareKeywords
# reads: kdbSQL -
#        driverSQL
# sets:  keywords - driver keywords that are not keywords in KDb
compareKeywords () {
  numFound=0
  for(( i=0; i < ${#driverSQL[@]}; i++ )) ; do
    found="no"
    for(( j=0; j < ${#kdbSQL[@]}; j++ )) ; do
      if [ "${driverSQL[$i]}" == "${kdbSQL[$j]}" ] ; then
        found="yes"
      fi
    done
    if [ "$found" == "no" ] ; then
      keywords[$numFound]="${driverSQL[$i]}"
      (( numFound++ ))
    fi
  done
}


# getDriverKeywords
# params : kdb -
#          driver -
#          outFile -
getDriverKeywords () {
  local kdb="$1"
  local driver="$2"
  local outFile="$3"

  declare -a kdbSQL
  declare -a driverSQL

  echo "Looking for driver-specific keywords in \"$driver\""
  readKeywords $kdb
  for(( i=0; i < ${#keywords[@]}; i++ )) ; do
    kdbSQL[$i]=${keywords[$i]}
  done
  unset keywords

  readKeywords $driver
  for(( i=0; i < ${#keywords[@]}; i++ )) ; do
    driverSQL[$i]=${keywords[$i]}
  done
  unset keywords

  compareKeywords
  echo "Writing driver-specific keywords for \"$driver\" to \"$outFile\""
  rm -f $outFile
  for(( i=0; i < ${#keywords[@]}; i++ )) ; do
    echo ${keywords[$i]} >> $outFile
  done
  unset keywords
}
################################################################################


################################################################################
# KDb lexer

checkKDbKeywords () {
  local scanner="../src/parser/KDbSqlScanner.l"
  if [ ! -r kdb.all -o "$scanner" -nt "kdb.all" ] ; then
    echo "Getting keywords"
    grep '^(\?"[a-zA-Z_0-9]' "$scanner" | \
       sed 's/(\?"\([^"]*\)"[^"]*/\1\n/g' | \
       awk '/^[a-zA-Z_0-9]+$/ {print $1;}' | 
       sort | uniq > "kdb.all"
    awk '/^[a-zA-Z_0-9]+$/ {print $1;}' kdb_keywords.txt >> "kdb.all"
  fi
}

################################################################################
# DB lexer functions
# These functions munge the extracted lexers from DBs and write the collected
# keywords to file

# getSQLiteKeywords
# params : inFile  - SQLite3 lexer file
#          outFile - all SQLite3 keywords
getSQLiteKeywords () {
  local inFile="$1"
  local outFile="$2"
  
  echo "Getting SQLite keywords ($inFile -> $outFile)"
  sed -n '/^static Keyword aKeywordTable/,/};/p' $inFile | \
    awk '/  { "[a-zA-Z_0-9]*"/ { print $2;}' | \
    sed 's/"\(.*\)".*/\1/g' > $outFile
}

getPostgreSQLKeywords () {
  local inFile="$1"
  local outFile="$2"

  echo "Getting PostgreSQL keywords ($inFile -> $outFile)"
  sed -n '/^static const ScanKeyword ScanKeywords/,/};/p' $inFile | \
    awk '/\t{"[a-zA-Z_0-9]*"/ { print $1;}' | \
    sed 's/.*"\(.*\)".*/\1/g' | tr 'a-z' 'A-Z' > $outFile
}

# getMySQLKeywords
# params : inFile  - MySQL lexer file
#          outFile - all MySQL keywords
getMySQLKeywords () {
  local inFile="$1"
  local outFile="$2"

  echo "Getting MySQL keywords ($inFile -> $outFile)"
  sed -n '/^static SYMBOL symbols/,/};/p' $inFile | \
    awk '/  { "[a-zA-Z_0-9]*"/ { print $2;}' | \
    sed 's/"\(.*\)".*/\1/g' > $outFile
}

################################################################################
# DB tarball functions
# These functions extract the lexer files from the DB source tarballs

# checkExtracted
# params : tarball - tarball containing backend DB source
#          file - file in tarball containing DB's lexer
checkExtracted () {
  local tarball="$1"
  local file="$2"

  if [ ! -r "$file" ] ; then
    echo "Getting file \"$file\" from \"$tarball\""
    tar -zxf "$tarball" "$file"
  fi
}

# checkTarballs
checkTarballs () {
  local pathInTar
  local appName
  local appVer

  # SQLite (native DB backend) keywords
  appName="SQLite"
  appVer=sqlite
  inFile="tokenize.c"
  filePrefix="sqlite"
  if [ ! -r "$appVer.all" ] || [ ! -r "$appVer.new" ] ; then
    getSQLiteKeywords "$inFile" "$appVer.all"
  fi
  if [ "$appVer.all" -nt "$appVer.new" ] ; then
    getDriverKeywords "kdb.all" "$appVer.all" "$appVer.new"
    header "const char* const ${appName}Driver::keywords" "${filePrefix}driver.h" "$inFile" "${filePrefix}keywords.cpp"
    body   "$appVer.new" "${filePrefix}keywords.cpp"
    footer "${filePrefix}keywords.cpp"
  fi

  ls mysql-*.tar.gz postgresql-*.tar.gz 2>/dev/null | while read tarball ; do
   case "$tarball" in
     mysql-4.1.[0-9\.]*.tar.gz)
       pathInTar="sql/lex.h"
       appName="MySql"
       filePrefix="mysql"
       appVer="${tarball%.tar.gz}"
       if [ ! -r "$appVer.all" ] || [ ! -r "$appVer.new" ] ; then
         checkExtracted "$tarball" "$appVer/$pathInTar"
         getMySQLKeywords "$appVer/$pathInTar" "$appVer.all"
	 rm -rf "$appVer"
       fi

       if [ "$appVer.all" -nt "$appVer.new" ] ; then
         getDriverKeywords "kdb.all" "$appVer.all" "$appVer.new"
         header "const char* const ${appName}Driver::keywords" "${filePrefix}driver.h" "$appVer/$pathInTar" "${filePrefix}keywords.cpp"
         body   "$appVer.new" "${filePrefix}keywords.cpp"
         footer "${filePrefix}keywords.cpp"
       fi
       ;;

     postgresql-base-7.4.[0-9\.]*.tar.gz)
       pathInTar="src/backend/parser/keywords.c"
       appName="pqxxSql"
       filePrefix="pqxx"
       appVer=`echo "${tarball%.tar.gz}" | sed 's/-base//'`
       if [ ! -r "$appVer.all" ] || [ ! -r "$appVer.new" ] ; then
         checkExtracted "$tarball" "$appVer/$pathInTar"
         getPostgreSQLKeywords "$appVer/$pathInTar" "$appVer.all"
	 rm -rf "$appVer"
       fi

       if [ "$appVer.all" -nt "$appVer.new" ] ; then
         getDriverKeywords "kdb.all" "$appVer.all" "$appVer.new"
         header "const char* const ${appName}Driver::keywords" "${filePrefix}driver.h" "$appVer/$pathInTar" "${filePrefix}keywords.cpp"
         body   "$appVer.new" "${filePrefix}keywords.cpp"
         footer "${filePrefix}keywords.cpp"
       fi
       ;;

     *)
       echo "Don't know how to deal with $tarball - ignoring"
       ;;
    esac
  done
}

checkKDbKeywords
src=`printf "src/parser/KDbSqlScanner.l\n"\
"   and tools/kdb_keywords.txt"`
header "const char* const KDbDriverPrivate::kdbSQLKeywords" "Driver_p.h" "$src" "keywords.cpp"
body "kdb.all" "keywords.cpp"
footer "keywords.cpp"

checkTarballs
wc -l *.all *.new | awk '{print $2" "$1}' |sort|awk '{print $1"\t"$2}'
