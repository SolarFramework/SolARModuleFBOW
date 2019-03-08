cmake_minimum_required(VERSION 3.7.2)

##################################################
set (VERSION_NUMBER "0.5.1")
project("SolARModuleFBOW")
set (SOURCES 
	src/SolARModuleFBOW.cpp
    src/SolARKeyframeRetrieverFBOW.cpp
)

set (HEADERS
	interfaces/SolARFBOWAPI.h
    interfaces/SolARModuleFBOW_traits.h
    interfaces/SolARKeyframeRetrieverFBOW.h
)

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/xpcf_SolARModuleFBOW_registry.xml" DESTINATION $ENV{BCOMDEVROOT}/.xpcf/SolAR/	)

##################################################

# various macros
include("$ENV{BCOMDEVROOT}/bcomBuild/SolARFramework/solarmacros.cmake")
# config setup
setup()
# process packagedependencies.txt
processPackagedependencies()
# define targets (type library)
defineTargets("library" "")


