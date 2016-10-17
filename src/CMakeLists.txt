add_executable(my-scheduler my-scheduler.cpp)
target_link_libraries(my-scheduler ${MESOS_LIBRARIES})

add_executable(my-executor my-executor.cpp)
target_link_libraries(my-executor ${MESOS_LIBRARIES} pthread)
#target_link_libraries(my-executor mesos_no_3rdparty state log process pthread)
#set_target_properties(my-executor PROPERTIES LINK_FLAGS -static)

