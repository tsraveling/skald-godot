#ifndef SKALD_RESPONSES_H
#define SKALD_RESPONSES_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
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

protected:
	static void _bind_methods();

public:
	SkaldContent() = default;
	~SkaldContent() = default;

	void set_attribution(const godot::String &p_attribution);
	void set_text(const godot::String &p_text);

	godot::String get_attribution() const;
	godot::String get_text() const;
};

class SkaldOptionGroup : public godot::RefCounted {
	GDCLASS(SkaldOptionGroup, godot::RefCounted)

	godot::Array options_;

protected:
	static void _bind_methods();

public:
	SkaldOptionGroup() = default;
	~SkaldOptionGroup() = default;

	void add_option(const godot::String &p_text, bool p_available);

	godot::Array get_options() const;
	int get_count() const;
};

class SkaldQuery : public godot::RefCounted {
	GDCLASS(SkaldQuery, godot::RefCounted)

	godot::String method_;
	godot::Array args_;

protected:
	static void _bind_methods();

public:
	SkaldQuery() = default;
	~SkaldQuery() = default;

	void set_method(const godot::String &p_method);
	void set_args(const godot::Array &p_args);

	godot::String get_method() const;
	godot::Array get_args() const;
	int get_arg_count() const;
	godot::Variant get_arg(int p_index) const;
};

class SkaldAction : public godot::RefCounted {
	GDCLASS(SkaldAction, godot::RefCounted)

	godot::String method_;
	godot::Array args_;

protected:
	static void _bind_methods();

public:
	SkaldAction() = default;
	~SkaldAction() = default;

	void set_method(const godot::String &p_method);
	void set_args(const godot::Array &p_args);

	godot::String get_method() const;
	godot::Array get_args() const;
	int get_arg_count() const;
	godot::Variant get_arg(int p_index) const;
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

class SkaldParseResult : public godot::RefCounted {
	GDCLASS(SkaldParseResult, godot::RefCounted)

	bool ok_ = true;
	godot::Array errors_;

protected:
	static void _bind_methods();

public:
	SkaldParseResult() = default;
	~SkaldParseResult() = default;

	void set_ok(bool p_ok);
	// Appends one parse error. severity: 0 = warning, 1 = error.
	void add_error(const godot::String &p_message, int p_line, int p_column,
			const godot::String &p_source, int p_severity);

	bool is_ok() const;
	godot::Array get_errors() const;
	int get_error_count() const;
};

class SkaldNotification : public godot::RefCounted {
	GDCLASS(SkaldNotification, godot::RefCounted)

	godot::String var_name_;
	godot::String scope_;
	godot::Variant value_;

protected:
	static void _bind_methods();

public:
	SkaldNotification() = default;
	~SkaldNotification() = default;

	void set_var_name(const godot::String &p_name);
	void set_scope(const godot::String &p_scope);
	void set_value(const godot::Variant &p_value);

	godot::String get_var_name() const;
	godot::String get_scope() const;
	bool has_value() const;
	godot::Variant get_value() const;
};

#endif // SKALD_RESPONSES_H
