# Custom Fighter
My take on creating a fully customizable fighting game engine, based on games from the genre
and MUGEN. My goal is to experiment using libraries I'm unfamiliar with and to create a unique 
game that can easily be modified and tested.

This repo contains the game itself, along the with tool used to create the base movelist for characters.

# Technologies
* [ggpo](https://github.com/pond3r/ggpo)
* [curl](https://github.com/curl/curl)
* [imgui](https://github.com/ocornut/imgui)
* [nativefiledialog](https://github.com/mlabbe/nativefiledialog)
* [json](https://github.com/nlohmann/json)
* [SFML](https://github.com/SFML/SFML)
* [sockpp](https://github.com/fpagliughi/sockpp)

#  Screenshots
### In-game
| ![alt text](https://i.imgur.com/OXpiHzh.png)  |
|:--:| 
| *Executing a command grab attack* |

| ![alt text](https://i.imgur.com/GFNvHCS.png) | 
|:--:| 
| *Selecting moveslist for character*  |

### Editor
| ![alt text](https://i.imgur.com/bkjquOW.png)  |
|:--:| 
| *Animation playback* |

| ![alt text](https://i.imgur.com/Bv5GvWy.png) |
| :--: |
| *Adjusting hitboxes* |

# Building & Installing
Requirements are already provided if you download the repo.

### Build
1. `mkdir build`
2. `cd build`
3. `cmake ../ -G "MinGW Makefiles"`
4. `make all`

### Install
5. Finally to complete the installation move the required executable files.

# File Structure
The detailing of the layout of the games assets.

## `./data/`
Game assets are all found within the game folder, everything except executables are located here.

## `./data/clothing`
The clothing, and base skin texture exist as listing entry folders here. The only required clothing entry 
is `skin` other clothing entries are entirely optional and only used to customize characters in-game.

*Note: File format is recommended to stay consistent all png, jpg, etc.

*Note: Clothing is display and scaled according the `skin` textures. Clothing entries will scale so that the 
pixel location of the lower layers match up and centered properly. It is recommended to create textures
with an overlay of the base skin texture to see how it lines up.

### Entry Files
- `head`
- `neck`
- `torsoFront`
- `torsoBack`
- `upperArm`
- `foreArm`
- `handFront`
- `handBack`
- `calf`
- `thigh`
- `foot`

### `./data/effects`
These are exploded animated gif frames, frames are loaded from [1,...n].

### List of used effects
- `blockspark`
- `blood`
- `dust`
- `hitspark`
- `sweat`

### `./data/fonts/`
These fonts are required to read text within the game, can be altered but GUI may not scale accordinly.

### `./data/gamemode/rounds_choices.json`
A unique gamemode where players can unlock new moves as rounds progress.

This contains a json formatted list containing the moves which can be unlocked.

```
[
    {
        "isDefault": true,              // Is a default move added to character at start
        "move": "Stand Light Punch",    // Name of move
        "motion": "A"                   // Input to be associated
    }
]
```

### `./data/moves/`
Moves are json formatted files that are uniquely named. These are attacks / stances characters can perform, and 
contain hitboxes (self), hurtboxes (damage others), cancels (mid-animation cancels via input or otherwise) and vertex
data for the skeletons. Moves are the bread and butter of the `editor.exe` where all attributes can be modified within. 
Moves can also access `./data/sounds/`, and other `./data/moves/` within them.

### `./data/sounds/`, `./data/musics/`
All sounds are formatted as wav, sounds already included are required for the game to function.
New sounds can be included and selected within the editor. 

### `./data/shaders/`
Shaders used for the outline of characters in-game, should not be modified but can be adjusted to change outline thickness.

### `./data/stages/`
Stages will be randomly selected when starting a new game. Stages are entry folders that contain separate images
for each layer. How the stage is rendered is defined within the `stage.json` file.

```
{
    "layers": [
        {
            "scale": 1,                 // Scale of the horizontal axis moving compared to the camera
            "texture": "planks.png"     // Texture used for this layer
        },
        {
            "scale": 0.5,
            "texture": "water.png"
        },
        ...
    ]
}
```
