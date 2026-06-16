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
    # A codex (optional) defines globals + methods and the project root.
    engine.setup("res://dialogue/project.codex")
    var result = engine.load("res://dialogue/intro.ska")
    if not result.ok:
        for err in result.errors:
            push_error("Parse error (line %d): %s" % [err.line, err.message])
        return
    handle(engine.start())

func handle(response):
    if response is SkaldContent:
        print(response.attribution, ": ", response.text)
        handle(engine.advance())                 # nothing to choose — advance
    elif response is SkaldOptionGroup:
        # Options are now their OWN response, not nested in content.
        for i in response.count:
            var opt = response.options[i]
            if opt.is_available:
                print("  [%d] %s" % [i, opt.text])
        # ... later, once the player picks: handle(engine.act(index))
    elif response is SkaldQuery:
        # A method call that EXPECTS a return value — reply with answer().
        var value = process_query(response.method, response.args)
        handle(engine.answer(value))
    elif response is SkaldAction:
        # A fire-and-forget method call — run it, then advance (do NOT answer).
        run_action(response.method, response.args)
        handle(engine.advance())
    elif response is SkaldNotification:
        print("set %s (%s) = %s" % [response.var_name, response.scope, response.value])
        handle(engine.advance())
    elif response is SkaldGoModule:
        engine.load(response.module_path)
        handle(engine.start_at(response.start_tag) if response.start_tag else engine.start())
    elif response is SkaldEnd:
        print("-- end --")
    elif response is SkaldError:
        push_error("Skald error %d: %s" % [response.code, response.message])

func _on_choice_selected(index: int):
    handle(engine.act(index))
```

You can also add `SkaldEngine` directly to your scene tree in the editor — it appears in the node list like any other node.

## API reference

### SkaldEngine (Node)

| Method | Description |
|---|---|
| `setup(path: String) -> SkaldParseResult` | Load a `.codex` project (globals, methods, project root). Optional. |
| `load(path: String) -> SkaldParseResult` | Parse a single `.ska` module. |
| `start() -> Variant` | Begin at the first block. Returns a response. |
| `start_at(tag: String) -> Variant` | Begin at a specific block tag. |
| `act(choice_index: int = 0) -> Variant` | Advance. Pass an option index for a `SkaldOptionGroup`, or `0` otherwise. |
| `advance() -> Variant` | Convenience for `act(0)` — advance past any non-choice response. |
| `get_current() -> Variant` | Re-read the most recent response without advancing. |
| `answer(value) -> Variant` | Reply to a `SkaldQuery`. Pass `int`, `float`, `String`, `bool`, or `null`. |
| `set_global(key: String, value) -> Variant` | Set a codex global. Returns `null`, or a `SkaldError`. |
| `get_global(key: String) -> Variant` | Get a codex global, or a `SkaldError`. |

`setup()` and `load()` return a **SkaldParseResult** (`ok: bool`, `errors: Array`, `error_count: int`); each entry in `errors` is a Dictionary with `message`, `line`, `column`, `source`, and `severity` (`0` = warning, `1` = error). Globals set via `set_global` / declared in the codex persist across module loads for the engine's lifetime.

Every method that returns `Variant` returns one of the response types below. Use `is` to branch:

```gdscript
if response is SkaldContent:
    ...
```

### Response types

**SkaldContent** — narrative text to display. (Options are no longer carried here — see `SkaldOptionGroup`.)

| Property | Type |
|---|---|
| `text` | `String` |
| `attribution` | `String` — speaker tag, or `""` |

**SkaldOptionGroup** — a set of choices, delivered as its own response.

| Property / Method | Type |
|---|---|
| `count` | `int` |
| `options` | `Array` of `SkaldOption` (`text: String`, `is_available: bool`) |

Present the options, then call `engine.act(index)` with the chosen index.

**SkaldQuery** — a method call that expects a return value (a *get*).

| Property / Method | Type |
|---|---|
| `method` | `String` |
| `args` | `Array` |
| `get_arg(index)` | `Variant` |

Respond with `engine.answer(value)` where `value` is the appropriate type, or `null`.

**SkaldAction** — a fire-and-forget method call (a *post*). Same shape as `SkaldQuery`, but it expects **no** return value. Handle the side effect, then `engine.act()` / `engine.advance()`. Calling `answer()` on an action returns a `RESOLUTION_QUEUE_EMPTY` error.

**SkaldNotification** — a variable was mutated; observe state changes here.

| Property / Method | Type |
|---|---|
| `var_name` | `String` |
| `scope` | `String` — `"global"`, `"module"`, or `"local"` |
| `has_value()` | `bool` |
| `value` | `Variant` — the resolved new value, or `null` |

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
