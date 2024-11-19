# Resource Roulette

<p align="center" width="100%">
<img alt="Icon" src="./ResourceRoulette.png" width="15%" />
</p>

# NOTE **Do not use with a world you are not willing to abandon**
This mod is still in it's early phases and updates in the near future will likely break saves, especially ensuring Multiplayer works properly and adding configuration options.

## Description

Beef's Resource Roulette allows you to randomize the locations of resources in the gameworld.

## Features
- Randomizes Resources in the world, and updates their visual features accordingly.
  - Includes ability to randomize all solid nodes and crude oil resources
  - Includes compatibility with modded resource nodes from Ficsit Farming and Refined Power and easy ability to add more

- Handles odd node locations resulting in angled resource nodes gracefully with automagically world-aligning logic

## Known Bugs
- I haven't gotten local multiplayer to work yet on my machine, so it's yet untested in Multiplayer/Dedicated.
- Crude oil and resource node meshes have grass spawning through them sometimes, this should be able to be resolved in roadmap later on

## Changelog
- Version: 1.0.1
  - Prevent a rare issue with radiation emitter spawning inside the player
- Version: 1.0.0
  - Initial Release

## Roadmap

- [ ] Set up configuration menu options (what nodes to randomize, what nodes should allow grouping together, max nodes per group)
- [X] Include "Purity exclusion zones" around spawn locations that keep pure and/or normal nodes out to increase starting difficulty and keep grasslands from turning into free-for-all resource world
- [ ] Remove grass around meshes/decals using RVT https://dev.epicgames.com/documentation/en-us/unreal-engine/runtime-virtual-texturing-in-unreal-engine?application_version=5.0 or similar (Thanks Phenakist)
- [ ] Mix up Geyser purities
- [ ] Mix up fracking nodes (same locations but change the resource type/purities)
- [ ] Investigate adding new fracking node locations and use raycasting magic to settle them?
- [ ] Add additional locations for resource nodes, since already auto-leveling nodes is achieved

## Credits

Mod created by TheRealBeef, and much thanks to Oukibt for their direct help and the ability to analyze their Resource Node Randomizer mod to understand how to even start https://github.com/oukibt/ResourceNodeRandomizer.
