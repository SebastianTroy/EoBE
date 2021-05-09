add_executable(Tests)

target_sources(Tests
    PUBLIC
    main.cpp
    TestCircularBuffer.cpp
    TestShape.cpp
    TestQuadTree.cpp
    TestRangeConverter.cpp
    TestRollingStatistics.cpp
    TestWindowedFrequencyStatistics.cpp
    TestWindowedRollingStatistics.cpp
)

target_include_directories(Tests
    PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Utility
)

target_link_libraries(Tests
    PRIVATE
    Catch2::Catch2
    Utility
)
