#ifndef SKALD_ENGINE_H
#define SKALD_ENGINE_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <memory>

namespace Skald {
class Engine;
}

class SkaldEngine : public godot::Node {
	GDCLASS(SkaldEngine, godot::Node)

	std::unique_ptr<Skald::Engine> engine_;
	godot::Variant current_response_;
	godot::String codex_path_;

protected:
	static void _bind_methods();

public:
	SkaldEngine();
	~SkaldEngine();

	void _ready() override;

	void set_codex_path(const godot::String &p_path);
	godot::String get_codex_path() const;

	godot::Variant setup(const godot::String &p_path);
	godot::Variant load(const godot::String &p_path);
	godot::Variant start();
	godot::Variant start_at(const godot::String &p_tag);
	godot::Variant act(int p_choice_index = 0);
	godot::Variant continue_();
	godot::Variant get_current();
	godot::Variant answer(const godot::Variant &p_value);

	godot::Variant set_global(const godot::String &p_key, const godot::Variant &p_value);
	godot::Variant get_global(const godot::String &p_key);
};

#endif // SKALD_ENGINE_H
