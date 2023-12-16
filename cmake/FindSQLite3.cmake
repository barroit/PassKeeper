set(SQLITE3_VERSION "sqlite-amalgamation-3440200")
set(SQLITE3_DOWNLOAD_URL "https://www.sqlite.org/2023/${SQLITE3_VERSION}.zip")
set(SQLITE3_ZIP_FILE "${EXTERN_DIR}/.cache/${SQLITE3_VERSION}.zip")
set(SQLITE3_SOURCE_DIRS "${EXTERN_DIR}/sqlite3")

if (NOT IS_DIRECTORY ${SQLITE3_SOURCE_DIRS})
	if (NOT EXISTS ${SQLITE3_ZIP_FILE})
		message(STATUS "downloading sqlit3...")
		file(DOWNLOAD ${SQLITE3_DOWNLOAD_URL} ${SQLITE3_ZIP_FILE})
	endif()

	execute_process(COMMAND unzip -qo ${SQLITE3_ZIP_FILE} -d ${EXTERN_DIR})
	execute_process(COMMAND mv ${EXTERN_DIR}/${SQLITE3_VERSION} ${SQLITE3_SOURCE_DIRS})
endif()

set(SQLITE3_INCLUDE_DIRS ${SQLITE3_EXTRACT_DIR}/${SQLITE3_VERSION})