project(yagf C CXX)
cmake_minimum_required(VERSION 2.6.0)
#set(QT_QMAKE_EXECUTABLE /usr/bin/qmake-qt4)
if (NOT DEFINED  ${CMAKE_INSTALL_PREFIX})
set(CPACK_INSTALL_PREFIX /usr/)
set (CMAKE_INSTALL_PREFIX /usr/)
endif()
set (LIB_PATH_SUFFIX )
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    if (EXISTS "${CMAKE_INSTALL_PREFIX}lib64")
        set(LIB_PATH_SUFFIX 64)
    endif (EXISTS "${CMAKE_INSTALL_PREFIX}lib64")
    set_property (GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
add_definitions( -DPRILIBRARY_PATH="${CMAKE_INSTALL_PREFIX}lib${LIB_PATH_SUFFIX}/yagf/")

# following 2 lines define where interface translations will be installed
set(QML_DESTINATION share/yagf/translations/)
set(QML_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
add_definitions( -DQML_INSTALL_PATH="${QML_INSTALL_PREFIX}${QML_DESTINATION}")

set(SOURCES src/main.cpp src/mainform.cpp src/qgraphicsinput.cpp src/utils.cpp src/qxtunixsignalcatcher.cpp src/spellchecker.cpp src/PageAnalysis.cpp
    src/SkewAnalysis.cpp src/core/rotationcropper.cpp src/qxtgraphicsproxywidget.cpp src/popplerdialog.cpp src/pdfextractor.cpp src/pdf2ppt.cpp src/ghostscr.cpp
    src/configdialog.cpp src/core/ccbuilder.cpp src/core/analysis.cpp src/qsnippet.cpp src/sidebar.cpp src/droplabel.cpp src/projectmanager.cpp src/settings.cpp
    src/texteditor.cpp src/tblock.cpp src/tpage.cpp src/tpagecollection.cpp src/scanner.cpp src/core/imageprocessor.cpp src/recognitiondialog.cpp
    src/langselectdialog.cpp src/busyform.cpp src/tiffimporter.cpp src/pdfthread.cpp src/QBusyIndicator.cpp src/recognizerwrapper.cpp
    src/core/binarize.cpp src/core/util.cpp src/core/qipblackandwhiteimage.cpp src/core/qipgrayscaleimage.cpp src/core/subimagepp.cpp)

set(HEADERS src/settings.h)

set(INT_SOURCES src/preload.c)

set(UIS src/mainform.ui src/popplerdialog.ui src/configdialog.ui src/langselectdialog.ui src/busyform.ui src/recognitiondialog.ui)

set(MOC_HEADERS src/mainform.h src/qxtunixsignalcatcher.h src/qxtunixscinternal.h src/qgraphicsinput.h src/qxtgraphicsproxywidget.h src/qxtgraphicsview.h
    src/popplerdialog.h src/pdfextractor.h src/configdialog.h src/core/ccbuilder.h src/sidebar.h
    src/droplabel.h src/projectmanager.h src/texteditor.h src/tpage.h src/tpagecollection.h src/scanner.h src/core/imageprocessor.h
    src/langselectdialog.h src/busyform.h src/tiffimporter.h src/pdfthread.h src/QBusyIndicator.h src/recognizerwrapper.h
    src/recognizerwrapperinternal.h src/recognitiondialog.h src/core/subimagepp.h)

set(yagf_RCCS src/yagf.qrc)

set(QM_FILES ${CMAKE_BINARY_DIR}/yagf_ru.qm ${CMAKE_BINARY_DIR}/yagf_fr.qm)


add_definitions(-Wall -g)

find_package(Qt4 4.6 REQUIRED)
find_package(ASPELL REQUIRED)
include_directories(${ASPELL_INCLUDE_DIR})

include(${QT_USE_FILE})
qt4_wrap_ui(UI_HEADERS ${UIS})
qt4_wrap_cpp(MOC_SRCS ${MOC_HEADERS})
qt4_add_resources(yagf_RCC_SRCS ${yagf_RCCS})

include_directories( ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src)
add_executable(yagf ${SOURCES} ${UI_HEADERS} ${MOC_SRCS} ${yagf_RCC_SRCS} ${QM_FILES})
add_library(xspreload SHARED ${INT_SOURCES})

target_link_libraries(xspreload ${CMAKE_DL_LIBS})
target_link_libraries(yagf ${QT_LIBRARIES} ${ASPELL_LIBRARIES})

add_custom_target(translations DEPENDS ${QM_FILES})
qt4_add_translation(${CMAKE_BINARY_DIR}/yagf_ru.qm ${PROJECT_SOURCE_DIR}/src/mainform.cpp ${PROJECT_SOURCE_DIR}/src/popplerdialog.cpp ${MOC_SRCS} ${UI_HEADERS} ${PROJECT_SOURCE_DIR}/src/translations/yagf_ru.ts)
qt4_add_translation(${CMAKE_BINARY_DIR}/yagf_fr.qm ${PROJECT_SOURCE_DIR}/src/mainform.cpp ${PROJECT_SOURCE_DIR}/src/popplerdialog.cpp ${MOC_SRCS} ${UI_HEADERS} ${PROJECT_SOURCE_DIR}/src/translations/yagf_fr.ts)
#QT4_CREATE_TRANSLATION(yagf_ru.qm src/mainform.cpp ${MOC_SRCS} ${UI_HEADERS} src/yagf_ru.ts)
#set(CMAKE_INSTALL_PREFIX /usr/)

install(TARGETS yagf xspreload RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_PATH_SUFFIX}/yagf)
install(FILES ${QM_FILES} DESTINATION ${QML_DESTINATION})
#install(FILES COPYING DESCRIPTION README AUTHORS ChangeLog DESTINATION share/yagf/)
install(FILES yagf.png DESTINATION share/pixmaps/)
install(FILES yagf.png DESTINATION share/icons/hicolor/96x96/apps/)
install(FILES YAGF.desktop DESTINATION share/applications/)
if (EXISTS /usr/share/appdata/)
        install(FILES YAGF.appdata.xml DESTINATION /usr/share/appdata/)
endif (EXISTS /usr/share/appdata/)

#set(CPACK_PACKAGE_FILE_NAME yagf)
set(CPACK_PACKAGE_VENDOR "Andrei Borovsky, anb@symmetrica.net")
set(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
set(CPACK_SYSTEM_NAME "i586")
set(CPACK_BINARY_DEB ON)
set(CPACK_BINARY_RPM ON)
set(CPACK_BINARY_STGZ ON)
set(CPACK_BINARY_TGZ OFF)
set(CPACK_BINARY_TZ OFF)
set(CPACK_BINARY_TBZ2 OFF)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical front-end for cuneiform and tesseract OCR tools")
set(CPACK_PACKAGE_VERSION 0.9.1)
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_STRIP_FILES bin/yagf bin/libyagfpreload)
set(CPACK_SOURCE_TBZ2 "OFF")
set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TZ "OFF")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR}/yagf-${CPACK_PACKAGE_VERSION}/;/")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_SOURCE_DIR}/DESCRIPTION)
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README)
set(DEBIAN_PACKAGE_SECTION "text processing")
set(CPACK_RPM_PACKAGE_LICENSE "GPL v.3.0")
set (CPACK_RPM_PACKAGE_REQUIRES "libqt > 4.5")
include(CPack)


