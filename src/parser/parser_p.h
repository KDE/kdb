/* This file is part of the KDE project
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KEXIDB_PARSER_P_H
#define KEXIDB_PARSER_P_H

#include <qvaluelist.h>
#include <qdict.h>
#include <qasciicache.h>
#include <qstring.h>

#include <kexidb/queryschema.h>
#include <kexidb/tableschema.h>
#include <kexidb/connection.h>
#include "parser.h"

namespace KexiDB {

class ParserPrivate
{
	public:
		ParserPrivate();
		~ParserPrivate();

		void clear();

		int operation;
		TableSchema *table;
		QuerySchema *select;
		Connection *db;
		QString statement;
		ParserError error;
		QAsciiCache<char> reservedKeywords;
		bool initialized : 1;
};


/*! Data used on parsing. @internal */
class ParseInfo
{
	public:
		ParseInfo(QuerySchema *query);
		~ParseInfo();

		//! collects positions of tables/aliases with the same names
		QDict< QValueList<int> > repeatedTablesAndAliases;

		QString errMsg, errDescr; //helpers
		QuerySchema *querySchema;
};

}

#endif
