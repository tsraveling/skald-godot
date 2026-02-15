# skald-godot

A GDExtension that brings the [Skald](https://github.com/tsraveling/skald) narrative scripting engine into Godot as a native node. Write branching dialogue and interactive fiction in `.ska` files, then drive them from GDScript with a simple load/start/act loop.

## Installation

Download the latest `skald-godot.zip` from [Releases](https://github.com/tsraveling/skald-godot/releases) and extract it into your Godot project root. You should end up with:

```
your-project/
  addons/
    skald/
      skald.gdextension
      bin/
        libskald_godot.macos.template_debug.universal.dylib
        libskald_godot.linux.template_debug.x86_64.so
        ...
```

Restart the Godot editor. `SkaldEngine` and the response types will be available immediately — no plugin activation needed.

## Quick start

```gdscript
var engine := SkaldEngine.new()

func _ready():
    add_child(engine)
    engine.load("res://dialogue/intro.ska")
    var response = engine.start()
    handle(response)

func handle(response):
    if response is SkaldContent:
        print(response.attribution, ": ", response.text)
        for i in response.option_count:
            if response.is_option_available(i):
                print("  [%d] %s" % [i, response.get_option_text(i)])
    elif response is SkaldQuery:
        # The script is calling a method you define — respond with answer()
        var result = process_query(response.method, response.args)
        handle(engine.answer(result))
    elif response is SkaldEnd:
        print("-- end --")
    elif response is SkaldError:
        push_error("Skald error %d: %s" % [response.code, response.message])

func _on_choice_selected(index: int):
    var response = engine.act(index)
    handle(response)
```

You can also add `SkaldEngine` directly to your scene tree in the editor — it appears in the node list like any other node.

## API reference

### SkaldEngine (Node)

| Method | Description |
|---|---|
| `load(path: String)` | Parse a `.ska` file. |
| `start() -> Variant` | Begin at the first block. Returns a response. |
| `start_at(tag: String) -> Variant` | Begin at a specific block tag. |
| `act(choice_index: int = 0) -> Variant` | Advance after a content response. Pass the player's choice index, or 0 to continue. |
| `get_current() -> Variant` | Re-read the most recent response without advancing. |
| `answer(value) -> Variant` | Reply to a query. Pass `int`, `float`, `String`, `bool`, or `null`. |

Every method that returns `Variant` returns one of the response types below. Use `is` to branch:

```gdscript
if response is SkaldContent:
    ...
```

### Response types

**SkaldContent** — narrative text to display.

| Property / Method | Type |
|---|---|
| `text` | `String` |
| `attribution` | `String` — speaker tag, or `""` |
| `option_count` | `int` |
| `get_option_text(index)` | `String` |
| `is_option_available(index)` | `bool` |

**SkaldQuery** — the script is calling an external method.

| Property / Method | Type |
|---|---|
| `method` | `String` |
| `args` | `Array` |
| `expects_response` | `bool` |
| `get_arg(index)` | `Variant` |

Respond with `engine.answer(value)` where `value` is the appropriate type, or `null`.

**SkaldExit** — the script hit an `EXIT` statement.

| Property / Method | Type |
|---|---|
| `has_value()` | `bool` |
| `value` | `Variant` — the exit argument, or `null` |

**SkaldGoModule** — the script wants to transition to another module.

| Property / Method | Type |
|---|---|
| `module_path` | `String` |
| `start_tag` | `String` |

Handle this by calling `engine.load()` with the new path, then `engine.start()` or `engine.start_at()`.

**SkaldEnd** — the script reached its natural conclusion. No properties.

**SkaldError** — something went wrong.

| Property / Method | Type |
|---|---|
| `code` | `int` |
| `message` | `String` |
| `line_number` | `int` |

## Supported platforms

Pre-built binaries are provided for:

- macOS (universal: Apple Silicon + Intel)
- Linux x86_64
- Windows x86_64

## Local development

### Prerequisites

- Python 3
- [SCons](https://scons.org/) (`brew install scons` or `pip install scons`)

### Build

```bash
git clone --recursive https://github.com/tsraveling/skald-godot.git
cd skald-godot
scons                            # debug build for current platform
scons target=template_release    # release build
```

This produces a shared library in `addons/skald/bin/`.

### Import into a Godot project

Copy (or symlink) the `addons/skald/` directory into your Godot project:

```bash
cp -r addons/skald /path/to/your-project/addons/
```

Or symlink for faster iteration during development:

```bash
ln -s $(pwd)/addons/skald /path/to/your-project/addons/skald
```

Restart the editor. `SkaldEngine` and all response types will be available immediately.

## License

MIT
