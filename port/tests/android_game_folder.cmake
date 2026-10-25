# Compile the Android importer's recognition rule with a desktop JDK and check it.
#   -DJAVAC=... -DJAVA=... -DSRC=<GameFolder.java> -DCHECK=<GameFolderCheck.java> -DWORK=<dir>
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/classes")
execute_process(COMMAND "${JAVAC}" -d "${WORK}/classes" "${SRC}" "${CHECK}" RESULT_VARIABLE status)
if(status)
    message(FATAL_ERROR "javac failed: ${status}")
endif()
execute_process(COMMAND "${JAVA}" -cp "${WORK}/classes" io.github.missingno7.speedyblupi.GameFolderCheck
    "${WORK}/trees" RESULT_VARIABLE status)
if(status)
    message(FATAL_ERROR "GameFolderCheck failed: ${status}")
endif()
