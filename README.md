# skald-godot

Skald implementation for Godot. Will eventually be an official app plugin. For now, it's an autoload file that you can fork and include directly as a submodule.

## Installation

1. Use this repo or, if you want to customize it, fork this repo and use that.
2. In terminal, navigate to the folder you would like the `skald-godot` project folder to live in, and enter this command:
	`git submodule add https://github.com/tsraveling/skald-godot.git`
3. This will download the `skald-godot` repo as a folder. You can then pull new versions down from origin by using `git` within the `skald-godot` folder. For now, commits on the `main` branch will be tagged with version numbers.
4. In your Godot project, register the `skald-godot/Godot.gd` file as an autoload.

## Usage

You can attach `Skald.gd` to a node and use it that way, or set it up as an autoload singleton. The remainder of this documentation will assume use of the autoload approach.

### Getting started

You can load a file by calling `Skald.load("path_to_compiled_json_file")`. Currently, Godot only supports loading compiled Skald files, which can be done using the [Skald command line tool](https://www.npmjs.com/package/skald-compile). This will reset internal state to the start of that Skald file.

To actually start the interaction, call `Skald.get_first(game_state)`. `game_state` is a simple dictionary with your initial values you would like to pass in to the Skald engine. Unlike in the testing applications, the testbeds in a live Skald integration are ignored (although they are accessible within `Skald.skald_object` should you choose to use them in your game).

This will return a `SkaldResponse` object (see below).

### Processing Choices

Pull the list of choices out of `skald_response.choices`, and present them to the player if there are any, or a "Continue" button if not. Record the choice's `real_index` property, and use it to retrieve the next Skald block:

```
var next_response = Skald.get_next(selected_choice_index, game_state)
```

In `get_next`, the index is either the `real_index` of the choice, or simply `0` if you are continuing. `game_state` is a SkaldState object. You can simply use the SkaldState object returned by the previous choice, or update its own `.state` property using your overarching game state for a more dynamic approach.

Finally, if that response has an `end_with` property, it means the script has come to an end. Attempting to continue after this will throw an error. It is up to you to handle the script end gracefully within your game.

## Class Reference

### SkaldResponse

`SkaldResponse` is a class accessible via `Skald.SkaldResponse`, which has the following properties:

- `content: SkaldContent` -- the content of the current block (see below)
- `choices: [SkaldChoice]` -- a list of choices (see below) if any are available; if absent, just allow the plalyer to continue.
- `updated_state: SkaldState` -- the updated state following the player's previous choice and the current block.
- `end_with`: if this is present, it means the script has ended. If no value (ie `END some_value`) is supplied with in the Skald script, it will simply be true. Otherwise, you can use the value to perform next actions as you see fit.

### SkaldContent

`SkaldContent` describes a *block* of Skald content, ie, a single paragraph by a single actor:

- `section_tag: String` -- the tag of the section you are in, ie, for a section `#start` this will be `start`. Mostly used for debug purposes
- `attribution_tag: String` -- the tag of the actor "speaking" this line. You are responsible for processing this as you see fit.
- `content: String` -- the content. Stored as a raw string. Feel free to implement markdown or bbcode on your end, and use it in the Skald script.

## SkaldChoice

An array of these will be returned by a SkaldResponse, if a choice is available. If no choices are available, it means the script is carrying on to the next block automatically. You can either loop through to show all available blocks at once, or implement a Continue button (or equivalent).

Each `SkaldChoice` has:

- `text: String` -- text to display for the choice.
- `real_index: int` -- index of the choice in the original list of choices. For now, unavailable choices are not sent to the frontend.

### SkaldState

Skald handles state via a simple dictionary. This can be abstracted out of the rest of your game state, or stand on its own. If a state variable is *set* in a Skald script (ie, `been_here = true`), that key will be added to the dictionary. If you try to mutate or query it, however, an error will result.

State is then returned as part of the SkaldResponse object (`response_object.updated_state`), and can be reapplied to larger game state there. If you need to know immediately when state is mutated, though, you can subscribe to the `did_update_state(key, val)` signal in the Skald node.