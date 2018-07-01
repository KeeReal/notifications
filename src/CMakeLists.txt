add_executable(app main.c config.c node.c parser.c notifications.c)
install(TARGETS app DESTINATION bin)