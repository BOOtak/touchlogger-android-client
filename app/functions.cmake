function(add_executable_pie_nonpie)
     list(GET ARGV 0 exec_name)
     list(REMOVE_AT ARGV 0)
     add_executable(${exec_name}
         ${ARGV}
         )

     set_target_properties(${exec_name}
         PROPERTIES
         LINK_FLAGS "-pie"
         COMPILE_FLAGS "-fPIE"
         )

     add_executable(${exec_name}-nonpie
         ${ARGV}
         )
 endfunction(add_executable_pie_nonpie)

 function(target_link_pie_nonpie)
     list(GET ARGV 0 exec_name)
     list(REMOVE_AT ARGV 0)

     target_link_libraries(${exec_name}
         ${ARGV}
         )

     if (TARGET ${exec_name}-nonpie)
         target_link_libraries(${exec_name}-nonpie
             ${ARGV}
             )
     endif()
 endfunction(target_link_pie_nonpie)

 function(target_compile_definitions_pie_nonpie)
     list(GET ARGV 0 exec_name)
     list(REMOVE_AT ARGV 0)

     target_compile_definitions(${exec_name}
         ${ARGV}
         )

     if (TARGET ${exec_name}-nonpie)
         target_compile_definitions(${exec_name}-nonpie
             ${ARGV}
             )
     endif()
 endfunction(target_compile_definitions_pie_nonpie)
