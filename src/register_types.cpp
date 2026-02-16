#include "register_types.h"
#include "skald_engine.h"
#include "skald_responses.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_skald_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<SkaldEngine>();
	ClassDB::register_class<SkaldOption>();
	ClassDB::register_class<SkaldContent>();
	ClassDB::register_class<SkaldQuery>();
	ClassDB::register_class<SkaldExit>();
	ClassDB::register_class<SkaldGoModule>();
	ClassDB::register_class<SkaldEnd>();
	ClassDB::register_class<SkaldError>();
}

void uninitialize_skald_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}

extern "C" {
GDExtensionBool GDE_EXPORT skald_init(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_skald_module);
	init_obj.register_terminator(uninitialize_skald_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
