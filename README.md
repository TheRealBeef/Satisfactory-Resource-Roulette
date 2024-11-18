# Resource Roulette

<p align="center" width="100%">
<img alt="Icon" src="./ResourceRoulette.png" width="15%" />
</p>

## Description

Beef's Resource Roulette allows you to randomize the locations of resources in the gameworld.

## Features
- Randomizes Resources in the world
  - Includes ability to randomize crude oil locations
  - Including compatibility with modded resource nodes from Ficsit Farming and Refined Power and easy ability to add more

- Handles odd node locations resulting in angled resource nodes gracefully with automagically world-aligning logic

## Known Bugs
- Crude oil and resource node meshes have grass spawning through them sometimes, this should be able to be resolved in roadmap later on

## Changelog
- Version: 1.0.0
  - Not yet released

## Roadmap

- [ ] Set up configuration menu options (what nodes to randomize, what nodes should allow grouping together, max nodes per group)
- [ ] Include "Purity exclusion zones" around spawn locations that keep pure and/or normal nodes out to increase starting difficulty and keep grasslands from turning into free-for-all resource world
- [ ] Remove grass around meshes/decals using RVT https://dev.epicgames.com/documentation/en-us/unreal-engine/runtime-virtual-texturing-in-unreal-engine?application_version=5.0 or similar (Thanks Phenakist)
- [ ] Mix up Geyser purities
- [ ] Mix up fracking nodes (same locations but change the resource type/purities)
- [ ] Investigate adding new fracking node locations and use raycasting magic to settle them?
- [ ] Add additional locations for resource nodes, since already auto-leveling nodes is achieved

## Credits

Mod created by TheRealBeef, and much thanks to Oukibt for their direct help and the ability to analyze their Resource Node Randomizer mod to understand how to even start https://github.com/oukibt/ResourceNodeRandomizer.
