# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find sqlite3
# Find the native SQLITE3 headers and libraries.
#
# SQLITE3_INCLUDE_DIRS	- where to find sqlite3.h, etc.
# SQLITE3_LIBRARIES	- List of libraries when using sqlite.
# SQLITE3_FOUND	- True if sqlite found.

# Look for the header file.
FIND_PATH(POSTGRESQL_INCLUDE_DIR NAMES postgresql/libpq-fe.h)

# Look for the library.
FIND_LIBRARY(POSTGRESQL_LIBRARY NAMES pq)

# Handle the QUIETLY and REQUIRED arguments and set PostgreSQL_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PostgreSQL DEFAULT_MSG POSTGRESQL_LIBRARY POSTGRESQL_INCLUDE_DIR)

# Copy the results to the output variables.
IF(PostgreSQL_FOUND)
        SET(POSTGRESQL_LIBRARIES ${POSTGRESQL_LIBRARY})
        SET(POSTGRESQL_INCLUDE_DIRS ${POSTGRESQL_INCLUDE_DIR})
ELSE(PostgreSQL_FOUND)
        SET(POSTGRESQL_LIBRARIES)
        SET(POSTGRESQL_INCLUDE_DIRS)
ENDIF(PostgreSQL_FOUND)

MARK_AS_ADVANCED(POSTGRESQL_INCLUDE_DIRS POSTGRESQL_LIBRARIES)
