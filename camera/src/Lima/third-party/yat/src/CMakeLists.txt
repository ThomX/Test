set (source_files
  Exception.cpp
  bitsstream/BitsStream.cpp
  bitsstream/Endianness.cpp
  file/FileName.cpp
  file/PosixFileImpl.cpp
  memory/MemBuf.cpp
  network/Address.cpp
  network/ClientSocket.cpp
  network/Socket.cpp
  network/SocketException.cpp
  plugin/PlugIn.cpp
  plugin/PlugInManager.cpp
  plugin/PlugInUnix.cpp
  system/PosixSysUtilsImpl.cpp
  threading/Barrier.cpp
  threading/Message.cpp
  threading/MessageQ.cpp
  threading/PosixThreadingImpl.cpp
  threading/SharedObject.cpp
  threading/SyncAccess.cpp
  threading/Task.cpp
  time/Time.cpp
  utils/Logging.cpp
  utils/String.cpp
  utils/StringTemplate.cpp
  utils/StringTokenizer.cpp
  utils/URI.cpp
)

include_directories(
  ../include
)

add_library (yat SHARED ${source_files})

install (DIRECTORY ../include/yat DESTINATION include/
  FILES_MATCHING PATTERN "*.h"
  PATTERN "*.tpp"
  PATTERN "*.i"
  PATTERN "CMakeFiles" EXCLUDE
  PATTERN "config-macosx.h" EXCLUDE
  PATTERN "config-win32.h" EXCLUDE
  PATTERN "WinNt*" EXCLUDE
)
install (TARGETS yat LIBRARY DESTINATION ${LIB_INSTALL_DIR})
