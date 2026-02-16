#ifndef SKALD_RESPONSES_H
#define SKALD_RESPONSES_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

class SkaldOption : public godot::RefCounted {
	GDCLASS(SkaldOption, godot::RefCounted)

	godot::String text_;
	bool is_available_ = true;

protected:
	static void _bind_methods();

public:
	SkaldOption() = default;
	~SkaldOption() = default;

	void set_text(const godot::String &p_text);
	void set_is_available(bool p_available);

	godot::String get_text() const;
	bool get_is_available() const;
};

class SkaldContent : public godot::RefCounted {
	GDCLASS(SkaldContent, godot::RefCounted)

	godot::String attribution_;
	godot::String text_;
	godot::Array options_;

protected:
	static void _bind_methods();

public:
	SkaldContent() = default;
	~SkaldContent() = default;

	void set_attribution(const godot::String &p_attribution);
	void set_text(const godot::String &p_text);
	void add_option(const godot::String &p_text, bool p_available);

	godot::String get_attribution() const;
	godot::String get_text() const;
	godot::Array get_options() const;
	int get_option_count() const;
};

class SkaldQuery : public godot::RefCounted {
	GDCLASS(SkaldQuery, godot::RefCounted)

	godot::String method_;
	godot::Array args_;
	bool expects_response_ = false;

protected:
	static void _bind_methods();

public:
	SkaldQuery() = default;
	~SkaldQuery() = default;

	void set_method(const godot::String &p_method);
	void set_args(const godot::Array &p_args);
	void set_expects_response(bool p_expects);

	godot::String get_method() const;
	godot::Array get_args() const;
	int get_arg_count() const;
	godot::Variant get_arg(int p_index) const;
	bool get_expects_response() const;
};

class SkaldExit : public godot::RefCounted {
	GDCLASS(SkaldExit, godot::RefCounted)

	godot::Variant value_;

protected:
	static void _bind_methods();

public:
	SkaldExit() = default;
	~SkaldExit() = default;

	void set_value(const godot::Variant &p_value);

	bool has_value() const;
	godot::Variant get_value() const;
};

class SkaldGoModule : public godot::RefCounted {
	GDCLASS(SkaldGoModule, godot::RefCounted)

	godot::String module_path_;
	godot::String start_tag_;

protected:
	static void _bind_methods();

public:
	SkaldGoModule() = default;
	~SkaldGoModule() = default;

	void set_module_path(const godot::String &p_path);
	void set_start_tag(const godot::String &p_tag);

	godot::String get_module_path() const;
	godot::String get_start_tag() const;
};

class SkaldEnd : public godot::RefCounted {
	GDCLASS(SkaldEnd, godot::RefCounted)

protected:
	static void _bind_methods();

public:
	SkaldEnd() = default;
	~SkaldEnd() = default;
};

class SkaldError : public godot::RefCounted {
	GDCLASS(SkaldError, godot::RefCounted)

	int code_ = 0;
	godot::String message_;
	int line_number_ = 0;

protected:
	static void _bind_methods();

public:
	SkaldError() = default;
	~SkaldError() = default;

	void set_code(int p_code);
	void set_message(const godot::String &p_message);
	void set_line_number(int p_line);

	int get_code() const;
	godot::String get_message() const;
	int get_line_number() const;
};

#endif // SKALD_RESPONSES_H
