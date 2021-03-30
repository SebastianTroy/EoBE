set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    Effectors/Effector.cpp
    Effectors/EffectorFilterMouth.cpp
    Effectors/EffectorProboscisMouth.cpp
    Effectors/EffectorSpike.cpp
    Effectors/EffectorSpringTail.cpp
    Effectors/EffectorTail.cpp
    Egg.cpp
    Entity.cpp
    EntityPropertyTableModel.cpp
    FeedDispenser.cpp
    FoodPellet.cpp
    Genome/ChromosomePair.cpp
    Genome/Gene.cpp
    Genome/GeneBrain.cpp
    Genome/GeneEffectorBase.cpp
    Genome/GeneEffectorFilterMouth.cpp
    Genome/GeneEffectorProboscisMouth.cpp
    Genome/GeneEffectorSpike.cpp
    Genome/GeneEffectorSpringTail.cpp
    Genome/GeneEffectorTail.cpp
    Genome/GeneFactory.cpp
    Genome/GenePigment.cpp
    Genome/GeneSenseBase.cpp
    Genome/GeneSenseLunarCycle.cpp
    Genome/GeneSenseMagneticField.cpp
    Genome/GeneSenseRandom.cpp
    Genome/GeneSenseSine.cpp
    Genome/GeneSenseTraitsBase.cpp
    Genome/GeneSenseTraitsInArea.cpp
    Genome/GeneSenseTraitsRaycast.cpp
    Genome/GeneSenseTraitsSelf.cpp
    Genome/GeneSenseTraitsTouching.cpp
    Genome/Genome.cpp
    InspectorPanel.cpp
    LineGraph.cpp
    LineGraphContainerWidget.cpp
    MeatChunk.cpp
    NeuralNetworkInspector.cpp
    QuadTree.cpp
    ScatterGraph.cpp
    Sensors/Sense.cpp
    Sensors/SenseLunarCycle.cpp
    Sensors/SenseMagneticField.cpp
    Sensors/SenseRandom.cpp
    Sensors/SenseSine.cpp
    Sensors/SenseTraitsBase.cpp
    Sensors/SenseTraitsInArea.cpp
    Sensors/SenseTraitsRaycast.cpp
    Sensors/SenseTraitsSelf.cpp
    Sensors/SenseTraitsTouching.cpp
    Spike.cpp
    Swimmer.cpp
    Universe.cpp
    UniverseWidget.cpp
)

set(PROJECT_HEADERS
    MainWindow.h
    Effectors/Effector.h
    Effectors/EffectorFilterMouth.h
    Effectors/EffectorProboscisMouth.h
    Effectors/EffectorSpike.h
    Effectors/EffectorSpringTail.h
    Effectors/EffectorTail.h
    Egg.h
    Entity.h
    EntityContainerInterface.h
    EntityPropertyTableModel.h
    FeedDispenser.h
    FoodPellet.h
    Genome/ChromosomePair.h
    Genome/Gene.h
    Genome/GeneBrain.h
    Genome/GeneEffectorBase.h
    Genome/GeneEffectorFilterMouth.h
    Genome/GeneEffectorProboscisMouth.h
    Genome/GeneEffectorSpike.h
    Genome/GeneEffectorSpringTail.h
    Genome/GeneEffectorTail.h
    Genome/GeneFactory.h
    Genome/GenePigment.h
    Genome/GeneSenseBase.h
    Genome/GeneSenseLunarCycle.h
    Genome/GeneSenseMagneticField.h
    Genome/GeneSenseRandom.h
    Genome/GeneSenseSine.h
    Genome/GeneSenseTraitsBase.h
    Genome/GeneSenseTraitsInArea.h
    Genome/GeneSenseTraitsRaycast.h
    Genome/GeneSenseTraitsSelf.h
    Genome/GeneSenseTraitsTouching.h
    Genome/Genome.h
    Genome/Phenotype.h
    InspectorPanel.h
    LineGraph.h
    LineGraphContainerWidget.h
    MeatChunk.h
    NeuralNetworkInspector.h
    QuadTree.h
    ScatterGraph.h
    Sensors/Sense.h
    Sensors/SenseLunarCycle.h
    Sensors/SenseMagneticField.h
    Sensors/SenseRandom.h
    Sensors/SenseSine.h
    Sensors/SenseTraitsBase.h
    Sensors/SenseTraitsInArea.h
    Sensors/SenseTraitsRaycast.h
    Sensors/SenseTraitsSelf.h
    Sensors/SenseTraitsTouching.h
    Spike.h
    Swimmer.h
    Universe.h
    UniverseParameters.h
    UniverseWidget.h
)

set(PROJECT_DESIGNER_FILES
    MainWindow.ui
    InspectorPanel.ui
)

set(PROJECT_RESOURCE_FILES
    Icons.qrc
    Images.qrc
)

qt_add_executable(Trilobytes
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_DESIGNER_FILES}
    ${PROJECT_RESOURCE_FILES}
)

target_compile_definitions(Trilobytes
    PUBLIC
    VERSION_MAJOR=0
    VERSION_MINOR=3
    VERSION_ADDITIONAL=alpha
)

target_include_directories(Trilobytes
    PRIVATE
    ${PROJECT_SOURCE_DIR}/Utility
)

target_link_libraries(Trilobytes
    PRIVATE
    Qt6::Widgets
    nlohmann_json::nlohmann_json
    fmt::fmt
    Utility
)

add_dependencies(Trilobytes
    Tests
)

add_custom_command(TARGET Trilobytes POST_BUILD
    COMMAND ${CMAKE_BINARY_DIR}/test/Tests
    COMMENT "Running Tests..."
)

if (WIN32)
    # Retrieve the absolute path to qmake and then use that path to find
    # the windeployqt and qtenv2 executables
    get_target_property(QMAKE_EXE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QMAKE_EXE}" DIRECTORY)
    find_program(WINDEPLOYQT_ENV_SETUP qtenv2.bat HINTS "${QT_BIN_DIR}")
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")

    add_custom_command(TARGET Trilobytes
        POST_BUILD
        COMMAND "${WINDEPLOYQT_ENV_SETUP}" && "${WINDEPLOYQT_EXECUTABLE}" \"$<TARGET_FILE:Trilobytes>\" --no-translations
        COMMENT "Packaging dependancies..."
    )
endif()

# Ensure our release contains our saved genomes
add_custom_command(TARGET Trilobytes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/SavedGenomes/ ${CMAKE_BINARY_DIR}/SavedGenomes/
    COMMENT "Copying SharedGenomes directory..."
)
