/*
 * Reaver - SQLite wrapper functions
 * Copyright (c) 2011, Tactical Network Solutions, Craig Heffner <cheffner@tacnetsol.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU General Public License in all respects
 *  for all of the code used other than OpenSSL. *  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so. *  If you
 *  do not wish to do so, delete this exception statement from your
 *  version. *  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 */

#include "sql.h"

sqlite3 *db = NULL;

int sql_init(void)
{
    int retval = 0;

    if(!db)
    {
        if(sqlite3_open(REAVER_DATABASE, &db) == 0)
        {
            retval = 1;
        }
    }

    return retval;
}

int sql_exec(char *query)
{
    int result = SQLITE_ERROR;

    if(query)
    {
        do
        {
            result = sqlite3_exec(db, query, NULL, NULL, NULL);
            usleep(BUSY_WAIT_PERIOD);
        } 
        while(result == SQLITE_BUSY);
    }

    return result;
}

char *sql_error_string(void)
{
    return (char *) sqlite3_errmsg(db);
}

void sql_cleanup(void)
{
    sqlite3_close(db);
    db = NULL;
}
