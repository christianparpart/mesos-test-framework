add_executable(my-scheduler my-scheduler.cpp)
target_link_libraries(my-scheduler ${MESOS_LIBRARIES})

add_executable(my-executor my-executor.cpp)
target_link_libraries(my-executor ${MESOS_LIBRARIES})

#if(ENABLE_STATIC)
#add_executable(mesostest_static main.cpp)
#target_link_libraries(mesostest_static ${MESOS_LIBRARIES})
#set_target_properties(mesostest_static PROPERTIES ENABLE_EXPORTS ON)
#install(TARGETS mesostest_static RUNTIME DESTINATION sbin)
#endif()
