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
- The total number of pure/normal/impure nodes in the world is identical to playing without this mod so it neither increases nor decreases availability of resources in the world, only how hard you have to work to find them (unless enabling Full Randomization)
- Handles odd node locations resulting in angled resource nodes gracefully with automagically world-aligning logic
- Configuration Options (Currently only applies to new sessions, will be addressed in future update)

## Configuration Options
### Seed Options (Not visible in New Game Savegame Settings, only in-game)
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
### Admin Options
- Click to Reset Nodes - Doesn't alter the seed value, but simply refreshes existing nodes and causes them to re-settle into the surrounding terrain.
  - Due to bugfixes, this can alter node types and randomization settings.
- Click to Prep Savegame for Mod Removal
  - If you intend to remove the mod from your savegame, click this button and then save the game to prevent CTD.
  - Removes all vanilla extractors from the modded nodes. The vanilla game doesn't have functionality to re-assign miners if the node changes. 

## Known Bugs/Issues
- Nodes visually shift as you get closer to them the first time, this is the result of automagically aligning them. This only occurs once per node in each save, when it is about 250m away from the player location. I'll look into smoothing this motion in the future.
- I haven't gotten local multiplayer to work yet on my machine, so it's yet untested in Multiplayer/Dedicated.
- Crude oil and resource node meshes have grass spawning through them sometimes, this should be able to be resolved in roadmap later on
- May have conflicts with some resource mods, please notify me if you find anything and I will add compatibility.

## How to safely remove this mod from your savegame
This can be used if you intend to remove the mod permanently, it can also be used to update a savegame from version 1.0.x to work with the 1.1.x and later versions properly
1. Load your Savegame with current version of Resource Roulette installed and enabled in SML
2. With the game loaded, open the Mod Savegame Settings (accessible via the mod's settings menu in the game)
3. At the bottom click the button labeled "Click to Prep Savegame for Mod Removal"
4. This will remove all vanilla extractors associated with custom nodes added by Resource Roulette
5. Save your game so the changes are also saved
6. In your mod loader, either deactivate the Resource Roulette mod or remove it entirely from the mod loader
7. Now, when you load the game without Resource Roulette, you will have no issues

## Changelog
- Version 1.1.8
  - Improve node "settling" logic on cliff surfaces to follow terrain contours
  - Fix extractors failing to re-associate with modded nodes
  - Prevent crude oil fracking cores from being orphaned from their satellites
  - Improved callback logic for spawns to ensure better compatibility with modded resources like BRN
  - Fix crude oil decals weren't always removed
- Version 1.1.7
  - Fix a "save leak" causing load times to increase over time.
  - Improved behavior of grouping setting to allow for larger "regions of resources" if desired
  - Increase Max Number of Nodes per Group to 8
  - Update for SML 3.10
- Version 1.1.6
  - Fix checkbox for Bauxite randomization missing causing it not to be randomized - Thanks Ryder17z for the report
  - Cleaned up edge case for node randomization processing causing a crash when clicking the re-roll button a lot.
- Version 1.1.5
  -  Fix fracking node extractor related crash - thanks EvilStudmuffin81 for the report
- Version 1.1.4
  - Due to bugfixing, randomization will change slightly. However, if you are happy with the distribution you currently have, the mod will support your existing randomization as well. If you want to update to the "proper" randomization, either re-roll or click "Reset nodes" 
  - Added custom profiling library to identify and improve performance. 
  - Optimized code, RR loading and re-rolling actions should be about 4x faster than previous version
  - Implemented logic for refreshing data cache on version update
  - Updated rescanning filtering logic
  - Introduced logic to identify and destroy "zombie" nodes that sometimes persisted even when mod was removed
  - Added logic to try and prevent creation of zombie nodes in the first place
  - Now destroys and manually respawns all nodes to improve handling of randomization
  - Refactored recursive grouping method to prevent edge-case errors
  - Fixed typo and resulting improper logic causing ignoring certain combinations of randomization settings on re-roll
  - If RR cannot associate an extractor to a node, now associates it with a custom "Invalid" node telling you to re-build the extractor
- Version 1.1.3
  - Add button under Admin Options to prep savegame for mod removal by removing extractors from modded nodes
  - Move update button to Admin Options to make it clear it's not needed in general
  - More reliable settings saving
- Version 1.1.2
  - Increase missed raycast tolerance to ensure nodes under blocking objects are settled.
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
- [ ] Add a button to prep saves for removal of the mod to prevent any potential crashes (Thanks Cora74!)
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

