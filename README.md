# Resource Roulette

<p align="center" width="100%">
<img alt="Icon" src="./ResourceRoulette.png" width="15%" />
</p>

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
### Seed Options (Not visible in New Game Savegame Settings, only in-game)
- Click to Update Nodes - Doesn't alter the seed value, but simple refreshes existing nodes and causes them to re-settle into the surrounding terrain.
  - This should be used in an existing save in order to ensure miners and nodes are properly centered, but as it has a 6% chance of causing non-grouped nodes to change their type, this is optional to not potentially break existing saves.
- Click To Reroll Nodes - Be careful, clicking this menu option rerolls immediately.
  - Resources are randomized according to the options chosen in the rest of options below
  - Resource randomization values are saved to your savegame so if you *do* accidentally click this, load a previous save to revert.
### Randomization Options
- Use Full Randomization - If used, ignores grouping/purity/distributions/exclusion zones and completely randomizes resources. This could theoretically result in a world only Uranium nodes that's impossible to complete, but is truly random.
- Purity Exclusion Zones - If used, tries to exclude higher-purity nodes from starting areas to increase challenge.
- Individual options to randomize specific resources (E.g. Uranium, SAM, etc)
### Grouping Options
- Max nodes per group - The maximum number of nodes that can be in a single "group". Default is 5
- Grouping Distance - For nodes that are close together, the radius to recursively search to find nodes that should be included into a "group" and have similar types and purities. 
  - Higher values result in groups having more nodes that are further apart. Very high values result in "regions" of specific resources
  - Lower values results in higher diversity and randomness among nodes.
- Individual options to group resources that are not grouped by default (E.g. Uranium, SAM, etc)

## Known Bugs/Issues
- Nodes visually shift as you get closer to them the first time, this is the result of automagically aligning them. This only occurs once per node in each save, when it is about 250m away from the player location. I'll look into smoothing this motion in the future.
- I haven't gotten local multiplayer to work yet on my machine, so it's yet untested in Multiplayer/Dedicated.
- Crude oil and resource node meshes have grass spawning through them sometimes, this should be able to be resolved in roadmap later on
- May have conflicts with some resource mods, please notify me if you find anything and I will add compatibility.

## Changelog
- Version 1.1.1
  - Added update resources button to session settings to refresh and re-settle resources, fixing the weird offsets in existing saves.
  - Fix unintended nondeterminisic behavior when re-rolling nodes. This means there is about a 6% chance that single nodes with no neighbors will change type if they were set by re-rolling.
  - Miners now properly snap to the center of nodes instead of offset, and should sit lower on angled nodes.
  - Altered raycasting and settling behavior to get more consistent and proper node settling.
- Version: 1.1.0
  - Requires a new save for proper functionality, future updates will no longer break saves
  - Moved all options to Session Settings
  - Improved randomization logic
  - Added re-roll functionality
  - Added "full randomization" functionality
  - Solved bugs related to reloading/randomization
  - Improved asset locations and sizing
  - Improved Compatibility with Buildable Resource Nodes Redux, Resource Node Creator in next patch
  - Fixed too-small interaction boxes for randomized nodes
- Version: 1.0.3
  - Add randomization and grouping configuration options. Set these on the main menu as changes to these settings will only apply to new sessions until re-roll functionality is added in a future update.
- Version: 1.0.2
  - Fix resource scanner not pinging nodes
- Version: 1.0.1
  - Prevent a rare issue with radiation emitter spawning inside the player
- Version: 1.0.0
  - Initial Release: Note - This mod is released for feedback and improvements and is intended to be used in Singleplayer, future updates are likely to change resource distribution and alter logic to ensure multiplayer works, so this should not yet be used with a world you are not willing to abandon and/or significantly restructure.

## Roadmap
- [X] Include "Purity exclusion zones" around spawn locations that keep pure and/or normal nodes out to increase starting difficulty and keep grasslands from turning into free-for-all resource world
- [X] Set up simple configuration menu options (what nodes to randomize, what nodes should allow grouping, grouping distance, and whether to use exclusion zones)
- [X] Port mod options to Mod Savegame settings to apply per-save, and add additional menu options (max nodes per group, manual seed, and re-roll function)
- [X] Add a full-random option that randomizes purities/resources without regard for playability (e.g. there can be a chance for all-Uranium world). (Thanks Innoro!)
- [ ] Remove grass around meshes/decals using RVT https://dev.epicgames.com/documentation/en-us/unreal-engine/runtime-virtual-texturing-in-unreal-engine?application_version=5.0 or similar (Thanks Phenakist!)
- [ ] Mix up Geyser purities
- [ ] Mix up fracking nodes (same locations but change the resource type/purities)
- [ ] Investigate adding new fracking node locations and use raycasting magic to settle them?
- [ ] Add additional locations for resource nodes, since already auto-leveling nodes is achieved
- [ ] Add support for https://ficsit.app/mod/AllMinable
- [X] Add support for ignoring Buildable Nodes Redux resources (Thanks sunyudai!)
- [ ] Add support for ignoring Resource Node Creator resources

## Credits

Much thanks to Oukibt for their direct help and the ability to analyze their Resource Node Randomizer mod to understand how to even start https://github.com/oukibt/ResourceNodeRandomizer.

