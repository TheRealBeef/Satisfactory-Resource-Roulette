# Resource Roulette

<p align="center" width="100%">
<img alt="Icon" src="./ResourceRoulette.png" width="15%" />
</p>

# NOTE: **Do not use with a world you are not willing to abandon**
This mod is still in it's early phases and updates in the near future will likely break saves, especially ensuring Multiplayer works properly and adding configuration options.

## Description
Beef's Resource Roulette allows you to randomize the locations of resource nodes in the gameworld.

## Features
- Randomizes Resources in the world, and updates their visual features accordingly.
  - Includes ability to randomize all solid nodes and crude oil resources
  - Includes compatibility with modded resource nodes from Ficsit Farming and Refined Power and easy ability to add more
- The total number of pure/normal/impure nodes in the world is identical to playing without this mod so it neither increases nor decreases availability of resources in the world, only how hard you have to work to find them
- Handles odd node locations resulting in angled resource nodes gracefully with automagically world-aligning logic
- Configuration Options (Currently only applies to new sessions, will be addressed in future update)

## Configuration Options
### Randomization Options
- Purity Exclusion Zones - If used, tries to exclude higher-purity nodes from starting areas to increase challenge
- Individual options to randomize specific resources (E.g. Uranium, SAM, etc)
### Grouping Options
- Grouping Radius - For nodes that are close together, the radius to recursively search to find nodes that should be included into a "group" and have similar types and purities. 
  - Higher values result in groups having more nodes that are further apart. Very high values result in "regions" of specific resources
  - Lower values results in higher diversity and randomness among nodes.
- Individual options to group resources that are not grouped by default (E.g. Uranium, SAM, etc)

## Known Bugs/Issues
- Nodes visually shift as you get closer to them the first time, this is the result of automagically aligning them. This only occurs once per node in each save, when it is about 250m away from the player location. I'll look into smoothing this motion in the future.
- I haven't gotten local multiplayer to work yet on my machine, so it's yet untested in Multiplayer/Dedicated.
- Crude oil and resource node meshes have grass spawning through them sometimes, this should be able to be resolved in roadmap later on
- Currently conflicts with "Buildable Nodes Redux" and potentially "All Minable" as well.

## Changelog
- Version: 1.0.3
  - Add simple configuration options. Set these on the main menu as they only apply to new sessions until re-roll functionality is added in a future update.
- Version: 1.0.2
  - Fix resource scanner not pinging nodes
- Version: 1.0.1
  - Prevent a rare issue with radiation emitter spawning inside the player
- Version: 1.0.0
  - Initial Release: Note - This mod is released for feedback and improvements and is intended to be used in Singleplayer, future updates are likely to change resource distribution and alter logic to ensure multiplayer works, so this should not yet be used with a world you are not willing to abandon and/or significantly restructure.

## Roadmap
- [X] Include "Purity exclusion zones" around spawn locations that keep pure and/or normal nodes out to increase starting difficulty and keep grasslands from turning into free-for-all resource world
- [X] Set up simple configuration menu options (what nodes to randomize, what nodes should allow grouping, grouping distance, and whether to use exclusion zones)
- [ ] Port mod options to Mod Savegame settings to apply per-save, and add additional menu options (max nodes per group, manual seed, and re-roll function)
- [ ] Add a full-random option that randomizes purities/resources without regard for playability (e.g. there can be a chance for all-Uranium world). (Thanks Innoro!)
- [ ] Remove grass around meshes/decals using RVT https://dev.epicgames.com/documentation/en-us/unreal-engine/runtime-virtual-texturing-in-unreal-engine?application_version=5.0 or similar (Thanks Phenakist!)
- [ ] Mix up Geyser purities
- [ ] Mix up fracking nodes (same locations but change the resource type/purities)
- [ ] Investigate adding new fracking node locations and use raycasting magic to settle them?
- [ ] Add additional locations for resource nodes, since already auto-leveling nodes is achieved
- [ ] Add support for https://ficsit.app/mod/AllMinable
- [ ] Add support for ignoring Buildable Nodes Redux resources (Thanks sunyudai!)

## Credits

Much thanks to Oukibt for their direct help and the ability to analyze their Resource Node Randomizer mod to understand how to even start https://github.com/oukibt/ResourceNodeRandomizer.
