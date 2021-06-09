# Trilobytes - Evolution Simulator
A life simulator, where each critter has a neural network brain, a set of sensors and a set of effectors.

All entities within the simulation can be detected only via senses and interacted with only via effectors.

Critters eat food to survive, and can reproduce a-sexually or sexually.

The senses and abilities a critter has are entirely based upon their genome, and all behaviour based on their neural network brain.

Senses and effectors can have their own mini neural network, so that specific behaviour can be inherited along with them.

Requirements
-----
Developed with Qt 6.0.0 & MinGW 8.1.0

Built with CMake and Ninja in QtCreator

The easiest way to build from sources yourself is to open project in QtCreator, select a compatible (C++17) toolchain and build through the QtCreator GUI.

TODO
-----
 - More GUI controlls
 - More niches
 - - More hazards
 - - More food types
 - - More environmental effects (localised friction? e.t.c.)
 - More senses
 - - Pedometer (distance travelled since last reset)
 - More effectors (body armor? reproductive organs? pheremone creators? poison creators? decoy creators? food creators? Egg layers?)
 - - Vortex generator, moves entities near to trilobyte in a vortex pattern
 - Genes that encode more than a single effector or sensor (Pair of antenna? pair of eyes? memory/flip-flop (loop output to input)?
 - Genes inspector view
 - Matrix based coordinate system/collisions
 - More entities (pheremones? poisons? more food types?)
 - tabs in left GUI to split up settings
 -  - Allow adding multiple trilobytes at a time
 -  - Move feed spawner
 -  - adjust feed spawner pellet energy/frequency
 -  - Add feed spawner
 -  - resize feed spawner
 - Image based drawing
 - - Replace placeholder images with coherently styled ones
 - - add images for each sense and effector
 - - add gene for image decals
 - add gene for egg size
 - allow laid eggs to be fertilised by passing trilobytes (optional, let genetics decide)
 - brain inspector needs ability to adjust line culling (i.e. don't show connections below x strength...) etc
 - Would be very insightful to record which mutations have been accumulated over the generations (could be useful to refine mutations and detect anything too lethal to be useful)
 - MAYBE "Aerodynamics" aspect of entities to control how they interact with friction, and a seperate momentum transform from location transform, so that facing & moving direction aren't always the same
 - - Allows for effectors that improve cornering etc
 - - Allows for certain effectors to change the gliding ability
 - - Allows for certain effectors to transition the trilobyte to a walker
 - Collision friction? 
 - - Each tick each entity checks for all colliding entities and applies their "collision friction" to itself
 - - A trilobytes own friction could be taken into account
 - - Allows for effectors that increase / decrease a trilobytes friction
 - - Further penalty for colliding with spikes
 - - Can add new entity types that add areas of friction, increasing environmental variety (-ve friction objects?)
 - Want a generic "spawner" type that can spawn in food pellets the same way they currently are, but also be used to spawn spikes etc, so that
 - Look into ChaiScript
 - - These would require exposing a lot of the C++ to ChaiScript so this should be a feature explored when the APIs are more stable
 - - - Could make an EntityChaiScript, so scripted entities  could exist
 - - - Could make an Sense/EffectorChaiScript, so scripted senses and abilities could be scripted
 - - - Could make a ChaiScriptGraph, so scripted graphs could be added
 - Tests for all Utility classes
 - Perhaps when zoomed way out and showing spawners, draw a large icon of the thing(s) they are spawning in the middle of their circle

Next Steps
----------
Revamp spawners and how they are interacted with, giving movement, size, speed, count... e.t.c. user controls
 - Spawn more than just food pellets?
 - - Toxins?
 - - Spikes/Rocks/Environmental hazards
 - - Requires

Ability to add/remove spikes/other entities manually

Needs more thought
------------------
Entity needs some generic introspection so that classes don't need to do a bunch of dynamic casts to see if they'll interact or not
   - Perhaps a pure virtual GetType() which returns Animal, Vegetable, or Mineral
   - Perhaps a pure virtual GetAlive() (Animal, Egg & MeatChunk are all animal, but MeatChunks are not alive)
   - There must be a way to do this in a single function, perhaps something akin to a tag system? Would allow entities to mix and match qualities

Need a seperate momentumTransform, which is not just affected by friction and input from tail etc but also factors in the direction faced, with perhaps EffectorDorsalFin or the like improving the efficiency of the conversion of the movement transform to match the facing transform
 - Add Effectors that increase / decrease friction
 - Add Effectors that change how quickly the direction faced becomes the direction of movement (also friction dependant)
 - Add global control for friction ((speed *= friction) so friction is between 0.0 and 1.0?)

More mouth types, or rework of how mouths work
 - Want to prevent evolving multiple mouths to get around consumption/rate limits
 - Want to prevent having one of each mouth type

Big Idea
---------
One Mouth/Tail... e.t.c.
 - 2D Phenotype map with circles representing (Filter mouth/ Toothed mouth/ suction mouth...), when phenotype point is within a circle it is allowed to hav
 - - Each circle represents a specific strategy, they can overlap, allowing traits of multiple strategies, but shouldn't overlap much? (or maybe they need to overlap almost to their centres to allow evolution? Perhaps some phenotype traits should be allowed to be entirely contained)
 - - Each circle will have a list of acceptible modifier genes
 - - Each phenotype could have more than one circle (define in one place but might be possible to have the same features in more than one place)
 - Basic gene for each that does default behaviour & owns neural network (which may do nothing, might want to require that the phenotype location is within a phenotypic circle somewhere)
 - - Need to work out how to enable the major basic phenotypes here without hard coding them, want to allow the addition of other phenotypes somehow
 - Phenotype location modifier genes that change the "location" of the organism phenotype for a particular part (e.g. mouth or tail... e.t.c.)
 - - Allows neat graphical representation showing where each swimmer is on the phenotype map
 - - The location will change which circle the phenotype lies within, must be within a circle somewhere to 
 - Genes that add functionality and/or modify basic functionality to the base gene
 - - (adding columns to the network will require a special type that knows the index of the column added for the gene, perhaps assign it in the constructor that takes a list of these things, and the constructor can set it)
 - Modding! If ChaiScript is used, then 
 - - Represent the phenotype landscape in json, and load it (plus mods from mod directory to allow custom genes to be included in phenotype circle whitelists)
 - - scrip-t should already have access to phenotype, should be able to just use basic scripted gene to do any of this
 - Complex phenotype entries for each, with construction modifiers and custom tick operations
 - - Idea is to filter which modifiers are allowed based on the phenotype map (circle(s) currently in allow these modifications?)
 - - Scale down modifications in response to how close they are to the centre of the nearest containing circle that allows them to have an effect
 
 - Mouth
 - - Feeding
 - - - Needs to include % chance food in mouth is caught (low efficiency filtering e.g.)
 - - - Needs to allow damage (teeth, overlapping jaw mouth perhaps?)
 - - - Needs to include damage dealt, energy extraction rate/efficiency

 - Tail
 - - Movement
 - - - Has usual tail, and stop/start/jump tail overlapping
 - - - Needs to include acceleration curve, energy store, jump trigger threshold
 
 - Fins (just one Fins per swimmer? or allow multiple fins in different places like real life?)
 - - Steering
 - - - Overlaps "legs" phenotype to right, to allow low energy high inertia movement to evolve seperately to tail
 - - - Overlaps "damage" phenotype to left, to allow "scalpel fish" type behaviour
