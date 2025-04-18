add_library(props2 props2.cpp props2.h util/strutils.cpp util/strutils.h)
set_property(TARGET props2 PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS props2 DESTINATION lib)
install(FILES props2.h DESTINATION include)
