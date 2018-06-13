#
#  libcontra - LIBrary for CONnection TRAcking
#
#  Copyright 2018 Thorsten Alteholz <libcontra@alteholz.eu>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program (see the file COPYING included with this
#  distribution); if not, write to the Free Software Foundation, Inc.,
#  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
ADD_SUBDIRECTORY(src) 
FIND_PACKAGE(Doxygen)

IF (DOXYGEN_FOUND)

SET(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/libcontra/Doxyfile.libcontra)
SET(DOXYGEN_OUTPUT ${CMAKE_SOURCE_DIR}/doc)

ADD_CUSTOM_COMMAND(
  OUTPUT ${DOXYGEN_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation n ..."
  COMMAND pwd
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/doc
  COMMAND mkdir ${CMAKE_SOURCE_DIR}/doc
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
  COMMAND ${CMAKE_COMMAND} -E echo "Done n."
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS ${DOXYGEN_INPUT}
  )

ADD_CUSTOM_TARGET(doc ALL DEPENDS ${DOXYGEN_OUTPUT})

ADD_CUSTOM_TARGET(doc_forced
  COMMAND ${CMAKE_COMMAND} -E echo_append "Building API Documentation f ..."
  COMMAND pwd
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/doc
  COMMAND mkdir ${CMAKE_SOURCE_DIR}/doc
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT}
  COMMAND ${CMAKE_COMMAND} -E echo "Done f."
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

ENDIF (DOXYGEN_FOUND)

