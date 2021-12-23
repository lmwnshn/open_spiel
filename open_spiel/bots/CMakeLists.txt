# WAN-CMAKE: Disable all bots except human bot to improve build speed.

if (${WAN_BUILD_FAST}) # WAN-CMAKE-IF

add_library (bots OBJECT
  human/human_bot.cc
  human/human_bot.h
)

add_subdirectory(human)

else() # WAN-CMAKE-ELSE

add_library (bots OBJECT
  gin_rummy/simple_gin_rummy_bot.cc
  gin_rummy/simple_gin_rummy_bot.h
  human/human_bot.cc
  human/human_bot.h
  uci/uci_bot.cc
  uci/uci_bot.h
)

add_subdirectory(gin_rummy)
add_subdirectory(human)
add_subdirectory(uci)

if (OPEN_SPIEL_BUILD_WITH_ROSHAMBO)
  add_subdirectory(roshambo)
endif()

if (OPEN_SPIEL_BUILD_WITH_XINXIN)
  add_subdirectory(xinxin)
endif()

endif() # WAN-CMAKE-ENDIF
