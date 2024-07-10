# CMake generated Testfile for 
# Source directory: /home/tserath/claude/mudoedit/version-next/99.working
# Build directory: /home/tserath/claude/mudoedit/version-next/99.working/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(appstreamtest "/usr/bin/cmake" "-DAPPSTREAMCLI=/usr/bin/appstreamcli" "-DINSTALL_FILES=/home/tserath/claude/mudoedit/version-next/99.working/build/install_manifest.txt" "-P" "/usr/share/ECM/kde-modules/appstreamtest.cmake")
set_tests_properties(appstreamtest PROPERTIES  _BACKTRACE_TRIPLES "/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;166;add_test;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;185;appstreamtest;/usr/share/ECM/kde-modules/KDECMakeSettings.cmake;0;;/home/tserath/claude/mudoedit/version-next/99.working/CMakeLists.txt;23;include;/home/tserath/claude/mudoedit/version-next/99.working/CMakeLists.txt;0;")
