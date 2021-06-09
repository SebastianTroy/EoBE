set(UTILITY_SOURCES
    ChromeTracing.cpp
    JsonHelpers.cpp
    NeuralNetwork.cpp
    NeuralNetworkConnector.cpp
    RangeConverter.cpp
    RollingStatistics.cpp
    Transform.cpp
    WindowedFrequencyStatistics.cpp
    WindowedRollingStatistics.cpp
)

set(UTILITY_HEADERS
    AutoClearingContainer.h
    Algorithm.h
    ChromeTracing.h
    CircularBuffer.h
    Energy.h
    FormatHelpers.h
    JsonHelpers.h
    MathConstants.h
    MinMax.h
    NeuralNetwork.h
    NeuralNetworkConnector.h
    QuadTree.h
    Random.h
    Range.h
    RangeConverter.h
    RollingStatistics.h
    Shape.h
    Transform.h
    TypeName.h
    WindowedFrequencyStatistics.h
    WindowedRollingStatistics.h
)

add_library(Utility OBJECT
    ${UTILITY_SOURCES}
    ${UTILITY_HEADERS}
)

target_link_libraries(Utility
    PRIVATE
    nlohmann_json::nlohmann_json
    fmt::fmt
)
