find_package(GTest CONFIG REQUIRED)

set( TEST_SOURCES
    main.cpp
    ECS/EntityManagerTests.cpp
    ECS/ComponentManagerTests.cpp
    ECS/WorldTests.cpp
)

add_executable( ECSTests ${TEST_SOURCES} )

target_link_libraries( ECSTests
    PRIVATE Engine
    PRIVATE GTest::gtest
)

set_target_properties( ECSTests PROPERTIES FOLDER "Tests" )

include( GoogleTest )
gtest_discover_tests( ECSTests )
