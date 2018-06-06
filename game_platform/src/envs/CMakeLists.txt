SET(HEADERS elements.h Agents.h FightTheLandlord.h
    )

SET(SOURCES elements.cpp Agents.cpp FightTheLandlord.cpp
    )

ADD_LIBRARY(LIBENVS STATIC ${HEADERS} ${SOURCES})
