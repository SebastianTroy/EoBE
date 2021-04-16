#ifndef UNIVERSEPARAMETERS_H
#define UNIVERSEPARAMETERS_H

/**
 * @brief The UniverseParameters class is meant to allow the tick methods to
 * easily obtain an expandable selection of user controlled settings, without
 * expanding the function signature repeatedly.
 *
 * These settings are things that interact with trilobytes, i.e. can be sensed or
 * have a direct effect on behaviour (physics, food spawning etc)
 *
 * TODO going forward use this to save/load user settings
 */
class UniverseParameters {
public:
    /// A value that can hopefully be used by trilobytes to coordinate behaviour
    /// through time
    double lunarCycle_ = 0.0;
    /// These mutations will result in behavioral/ability changes but will not
    /// affect the structure of the chromosomes, so will not affect breeding
    /// compatibility.
    /// The parameters are used to create a gaussian curve, from which the real
    /// count is taken from.
    double meanGeneMutationCount_ = 1.0;
    double geneMutationCountStdDev_ = 5.0;
    /// These mutations are to the chromososmes that comprise the genome and
    /// will rarely confer changes in ability, but will affect breeding
    /// compatibility
    /// The parameters are used to create a gaussian curve, from which the real
    /// count is taken from.
    double meanStructuralMutationCount_ = 0.8;
    double structuralMutationCountStdDev_ = 0.2;
    /// This adjusts the spawn rate for all food spawners
    double foodSpawnRateModifier = 1.0;
};

#endif // UNIVERSEPARAMETERS_H
