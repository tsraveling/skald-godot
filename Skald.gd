extends Node

signal skald_signal
signal did_update_state(key, val)

## The primary Skald node, used to run Skald JSON files.
##
## Contains logic to walk through a Skald script in JSON format, including file loading.


# HELPER CLASSES #

## Used to encapsulate skald state, including where the player is at, and specific inputs.
## Will often be initialized by the client in order to send to the Skald node, using a
## reactive (ie, state and mutation-based) data flow model.
class SkaldState:
	
	## Current index of the active Skald section
	var section: int
	
	## Index of the current block within the current section
	var block: int
	
	## Game state -- should be a distillation of overall game state. External mutation
	## should be handled from within the client.
	var state: Dictionary

	## Inits state object with section index, block index, and state dictionary	
	func _init(section: int = 0,block: int = 0,state: Dictionary = {}):
		self.section = section
		self.block = block
		self.state = state
	

## Used to encapsulate actual content to display to the user. This class should only be 
## generated by the Skald node itself, not by the client.
class SkaldContent:
	
	## Recorded for debug purposes
	var section_tag: String
	
	## Tag representing the character. Should be interpreted by client, e.g.
	## to show a formatted name, portrait, or icon
	var attribution_tag: String
	
	## The text the client should render.
	var content: String
	
	func _init(attribution_tag: String,content: String,section_tag: String):
		# NEXT: Swap this out with explicit strings, then perform injections etc.
		# back in the calling location.
		self.attribution_tag = attribution_tag
		self.content = content
		self.section_tag = section_tag


## Used to send text options to the client side. Send back to the Skald node in order to
## process the next set of logic and choices
class SkaldChoice:
	var text: String
	var real_index: int
	

## Used to encapsulate data sent back to the client from the Skald node, based checked the internal
## logic of the Skald file that is currently loaded. Should only be returned by the Skald node,
## not initialized by teh client.
class SkaldResponse:
	var content : SkaldContent
	var choices : Array
	var updated_state: SkaldState
	var end_with # Can be int, string, or bool. `true` by default.
	
	func _init(content : SkaldContent, \
			choices: Array, \
			updated_state: SkaldState, \
			end_with):
		self.content = content
		self.choices = choices
		self.updated_state = updated_state
		self.end_with = end_with


# VARS #

## Stores the loaded file path.
var file_name: String = ""

## Stores the parsed JSON content of the Skald file.
var skald_object: Dictionary = {}

## Regex object
var _injection_regex: RegEx


# PUBLIC INTERFACE


## This function is called to get the next content block. It will automatically process
## logic, bypass logical blocks, and perform transitions. It returns a SkaldResponse object,
## with text content to display, sometimes choices, and an updated state obejct. The client is
## responsible for storing this state object and potentially modifying it external to the Skald
## script itself. Provide the selected SkaldChoice if there is one, or null if not.
func get_next(choice_index: int, state: SkaldState):
	
	# First grab the section
	var section = skald_object.sections[state.section]
	if section == null:
		err("Invalid section at %d" % state.section)
	
	# Get available choices
	var choices = _available_choices(state, section)
	
	# If there are choices, process the one the user picked and go from there
	if choices:
		var choice = choices[choice_index]
		
		# Apply mutations of this choice
		_process_meta(choice.meta, state)
		
		# Also immediately apply the transition of this choice
		if not _process_transition(choice.meta, state):
			_step_forward(state, section)
	else:
		var block = section.blocks[state.block]
		if block == null:
			err("Invalid block at %d, %d" % [state.section, state.block])
		
		# We process transitiion here, after the user has applied input. Mutations etc. were
		# applied when the text was displayed.
		if not _process_transition(block.meta, state):
			_step_forward(state, section)
	
	# Now continue checked
	return _process_from(state)
	
	
## This function is called to get an "initial node", ie, to jump into a particular section,
## or just straight into the beginning of the Skald script (aka, the first section) if no
## start_tag argument is provided.
func get_first(game_state: Dictionary, start_tag = null) -> SkaldResponse:
	var state = SkaldState.new()
	if start_tag:
		state.section = _find_section_index(start_tag)
		if state.section < 0:
			err("Section tag not found: %s" % start_tag)
			return null
	else:
		state.section = 0
	state.block = 0
	state.state = game_state
	
	return _process_from(state)


# INTERNAL LOGIC #

## Applies the meta object to the state. Does not check conditions. Does not process
## transitions any more.
func _process_meta(meta: Dictionary, state: SkaldState):
	
	# Process mutations
	for mutation in meta.mutations:
		match(mutation.operator):
			"=":
				if typeof(mutation.value) == TYPE_STRING and mutation.value == "!": # Perform a boolean flip
					state.state[mutation.input] = !state.state[mutation.input]
				else: # Otherwise just set it to the given value
					state.state[mutation.input] = mutation.value
			"+=":
				state.state[mutation.input] += mutation.value
			"-=":
				state.state[mutation.input] -= mutation.value
		emit_signal("did_update_state", mutation.input, state.state[mutation.input])
	
	# Signals
	for sig in meta.signals:
		if sig.has('value'):
			emit_signal("skald_signal", sig['signal'], sig['value'])
		else:
			emit_signal("skald_signal", sig['signal'])


## Processes transition. Returns true if there is one, false if there isn't.
func _process_transition(meta: Dictionary, state: SkaldState):
	if meta.has('transition'):
		state.section = _find_section_index(meta.transition)
		state.block = 0
		if state.section < 0:
			err("Tag not found for transition: %s" % meta.transition)
			return false
		return true
	return false
		

func _choices_from_block(section: Dictionary, state: SkaldState):
	var choices = []
	for i in len(section.choices):
		var choice = section.choices[i]
		if not _check_conditions(choice.meta, state):
			continue
		var new_choice = SkaldChoice.new()
		new_choice.text = _process_text_content(choice.body, state)
		new_choice.real_index = i
		choices.append(new_choice)
	return choices
	
	
func _available_choices(state: SkaldState, section: Dictionary):
	if not _can_make_choice(state, section):
		return null
	return section.choices
	

func _can_make_choice(state: SkaldState, section: Dictionary):
	var how_many = len(section.blocks)
	
	# If there are no choices, show continue
	if len(section.choices) == 0:
		return false
	
	# If this is the last block, show choices
	if state.block == how_many - 1:
		return true
		
	# Otherwise, show continue if there are valid choices after this
	for i in range(state.block + 1, how_many):
		if _check_conditions(section.blocks[i].meta, state):
			return false
	
	# We are good to go, show choices
	return true
	

## Steps the state forward once.
func _step_forward(state, section):
	state.block += 1
	if state.block >= len(section.blocks):
		state.block = 0
		state.section += 1
		if state.section >= len(skald_object.sections):
			err("Reached unexpected EOF.")
			return


## Process a line of text with inline functions like input injection		
func _process_text_content(text: String, state: SkaldState):
	var ret = text
	for injection_match in _injection_regex.search_all(ret):
		var match_string = injection_match.get_string()
		var key = match_string.substr(1, len(match_string) - 2)
		ret = ret.replace(match_string, state.state[key])
	return ret

## Create Skald content from a given block
func _get_skald_content(block: Dictionary, section: Dictionary, state: SkaldState):
	if (block.type != "attributed"):
		return null
		
	var text = _process_text_content(block.body, state)
	
	return SkaldContent.new(block.tag, text, section.tag)


## This will carry forward until the next piece of content (ie, an attributed block)
## is found. If it is the last attributed block in the section, any choices checked that section
## will be returned as well. If there are no choices, the player will automatically
## transition to the next section in sequence.
func _process_from(state: SkaldState) -> SkaldResponse:
	
	# Now loop until we are ready to return a response:
	while true:
		
		# First get the actual section and block
		var section = skald_object.sections[state.section]
		var block = section.blocks[state.block]
		if section == null:
			err("Invalid Skald section at %d" % state.section)
			return null
		if block == null:
			err("Invalid Skald block at section %d, block %d" % [state.section, state.block])
			return null
		
		# Then we check the conditions for this block
		var skip_auto_transition = false
		var should_return = false
		if _check_conditions(block.meta, state):
			
			# Process the logic of the block (without processing transition)
			_process_meta(block.meta, state)
			
			# If this is an attributed block, we should return
			if block.type == "attributed":
				should_return = true
			elif block.type == "logic":
				# Logic ones should apply transition now
				skip_auto_transition = _process_transition(block.meta, state)
			
			# If this is the end, we should return and not auto transition
			if block.meta.has('endWith'):
				skip_auto_transition = true
				should_return = true
				
		# If this is the last block and there are choices, we should return
		# and also skip the auto transition
		var has_choices = _can_make_choice(state, section)
		if has_choices:
			skip_auto_transition = true
			should_return = true
		
		# If it's time to return, set up the return object. In this case the step forward
		# will be handled by get_next.
		if should_return:
			return SkaldResponse.new(
					_get_skald_content(block, section, state),
					_choices_from_block(section, state) if has_choices else [],
					state,
					block.meta.endWith if block.meta.has('endWith') else null)
		else:
			# If we have gotten this far, proceed to the next block
			if not skip_auto_transition:
				_step_forward(state, section)
	return null
	

## Checks a condition (input / value / operator) array. All conditions must be met to pass.
func _check_conditions(meta: Dictionary, state: SkaldState):
	for condition in meta.conditions:
		if not state.state.has(condition.input):
			warn("Checking for input not in state: %s" % condition.input)
			return false
		
		var current_value = state.state[condition.input]
		
		if typeof(condition.value) != typeof(current_value):
			warn("Type mismatch for input: %s (%s in state, %s in condition)"
					% [condition.input, typeof(current_value), typeof(condition.value)])
			return false
		
		match(condition.operator):
			"==":
				if condition.value != current_value:
					return false
			"!=":
				if condition.value == current_value:
					return false
			">":
				if current_value <= condition.value:
					return false
			"<":
				if current_value >= condition.value:
					return false
			">=":
				if current_value < condition.value:
					return false
			"<=":
				if current_value > condition.value:
					return false
	return true


func _find_section_index(tag: String):
	if skald_object.sections == null:
		err("Invalid skald file, aborting.")
		return -1
	
	for i in len(skald_object.sections):
		if skald_object.sections[i].tag == tag:
			return i
	
	warn("Warning: No section found with that tag: " + tag)
	return -1
	

# FILE OPERATIONS #

## Pass a Godot resource path to this method in order to load the Skald JSON file and set
## the engine up at the start of the script. Will throw errors if JSON is invalid, or if
## Skald file is empty.
func load(_filename):
	if not FileAccess.file_exists(_filename):
		err("File does not exist")
		return
	
	# Load the contents of the file
	file_name = _filename
	var file = FileAccess.open(_filename, FileAccess.READ)
	var contents = file.get_as_text()
	
	# Parse the data out of it
	skald_object = JSON.parse_string(contents)
	if skald_object == null:
		err("Could not parse JSON: " + _filename)
		return

	# Get the first section (if there is one)
	if len(skald_object.sections) < 1:
		err("Skald file has no sections, aborting")
		return
		

## Override these to implement custom logging
func err(msg: String):
	print("SKALD ERROR: %s" % msg)
	
func warn(msg: String):
	print("SKALD WARN: %s" % msg)

# Called when the node enters the scene tree for the first time.
func _ready():
	_injection_regex = RegEx.new()
	_injection_regex.compile("{[a-zA-Z0-9_]+}")
