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
 - Need to display some properties about the sim in another tab on the inspector side
 - - Requires moving the Entity stuff into its own designer-widget class
 - Need to refactor Swimmer into Trilobyte
 - Need to move QuadTree into utility, which will require some simple base class for entity to extend
 - - Then particle can also extend new base class, (FeedDispenser could also be a particle?)
 - More GUI controlls
 - More niches
 - - More hazards
 - - More food types
 - - More environmental effects (localised friction? e.t.c.)
 - More effectors (body armor? reproductive organs? pheremone creators? poison creators? decoy creators? food creators? Egg layers?)
 - Genes that encode more than a single effector or sensor (Pair of antenna? pair of eyes? memory/flip-flop (loop output to input)?
 - Improve rendering (image based? decals? OpenGL?)
 - Genes inspector view
 - More graphs & configurable graphs
 - Matrix based coordinate system/collisions
 - More entities (pheremones? poisons? more food types?)
 - tabs in left GUI to split up settings
 -  - Allow adding multiple swimmers at a time
 -  - Move feed dispenser
 -  - adjust feed dispenser pellet energy/frequency
 -  - Add feed dispenser
 -  - resize feed dispenser
 - add images for each sense and effector and body
 - add gene for image decals
 - add gene for egg size
 - allow laid eggs to be fertilised by passing swimmers (optional, let genetics decide)
 - brain inspector needs ability to adjust line culling (i.e. don't show connections below x strength...) etc
 - Would be very insightful to record which mutations have been accumulated over the generations (could be useful to refine mutations and detect anything too lethal to be useful)
 - MAYBE "Aerodynamics" aspect of entities to control how they interact with friction, and a seperate momentum transform from location transform, so that facing & moving direction aren't always the same
 - - Allows for effectors that improve cornering etc
 - - Allows for certain effectors to change the gliding ability
 - - Allows for certain effectors to transition the swimmer to a walker

Next Steps
----------
Tests for all Utility classes

Entity needs some generic introspection so that classes don't need to do a bunch of dynamic casts to see if they'll interact or not
   - Perhaps a pure virtual GetType() which returns Animal, Vegetable, or Mineral
   - Perhaps a pure virtual GetAlive() (Animal, Egg & MeatChunk are all animal, but MeatChunks are not alive)
   - There must be a way to do this in a single function, perhaps something akin to a tag system? Would allow entities to mix and match qualities

Now we have upgraded to Qt 6, look into Qt 3D (for realtime 2D graphical aspect)

Need a seperate momentumTransform, which is not just affected by friction and input from tail etc but also factors in the direction faced, with perhaps EffectorDorsalFin or the like improving the efficiency of the conversion of the movement transform to match the facing transform
 - Add Effectors that increase / decrease friction
 - Add Effectors that change how quickly the direction faced becomes the direction of movement (also friction dependant)
 - Add global control for friction

Add 3 mouth types,
 - ToothedMouth (circle touching body somewhere, pellets @ ~50% & eggs/meat chunks at ~75%, can bite swimmers for damage) perhaps it can only eat a fixed ammount per tick, with smaller bites being more energy efficient but doing less damage?

Add some effectors
 - Vortex generator, moves entities near to swimmer in a vortex pattern

Add adjustable friction cooeficient (speed *= friction) so friction is between 0.0 and 1.0

Ability to add/remove spikes
