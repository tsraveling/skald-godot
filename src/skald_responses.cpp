#include "skald_responses.h"

using namespace godot;

// --- SkaldOption ---

void SkaldOption::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_text"), &SkaldOption::get_text);
	ClassDB::bind_method(D_METHOD("get_is_available"), &SkaldOption::get_is_available);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_available"), "", "get_is_available");
}

void SkaldOption::set_text(const String &p_text) { text_ = p_text; }
void SkaldOption::set_is_available(bool p_available) { is_available_ = p_available; }

String SkaldOption::get_text() const { return text_; }
bool SkaldOption::get_is_available() const { return is_available_; }

// --- SkaldContent ---

void SkaldContent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_attribution"), &SkaldContent::get_attribution);
	ClassDB::bind_method(D_METHOD("get_text"), &SkaldContent::get_text);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "attribution"), "", "get_attribution");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "", "get_text");
}

void SkaldContent::set_attribution(const String &p_attribution) { attribution_ = p_attribution; }
void SkaldContent::set_text(const String &p_text) { text_ = p_text; }

String SkaldContent::get_attribution() const { return attribution_; }
String SkaldContent::get_text() const { return text_; }

// --- SkaldOptionGroup ---

void SkaldOptionGroup::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_options"), &SkaldOptionGroup::get_options);
	ClassDB::bind_method(D_METHOD("get_count"), &SkaldOptionGroup::get_count);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "options"), "", "get_options");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "count"), "", "get_count");
}

void SkaldOptionGroup::add_option(const String &p_text, bool p_available) {
	Ref<SkaldOption> option;
	option.instantiate();
	option->set_text(p_text);
	option->set_is_available(p_available);
	options_.push_back(option);
}

Array SkaldOptionGroup::get_options() const { return options_; }
int SkaldOptionGroup::get_count() const { return options_.size(); }

// --- SkaldQuery ---

void SkaldQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_method"), &SkaldQuery::get_method);
	ClassDB::bind_method(D_METHOD("get_args"), &SkaldQuery::get_args);
	ClassDB::bind_method(D_METHOD("get_arg_count"), &SkaldQuery::get_arg_count);
	ClassDB::bind_method(D_METHOD("get_arg", "index"), &SkaldQuery::get_arg);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "method"), "", "get_method");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "args"), "", "get_args");
}

void SkaldQuery::set_method(const String &p_method) { method_ = p_method; }
void SkaldQuery::set_args(const Array &p_args) { args_ = p_args; }

String SkaldQuery::get_method() const { return method_; }
Array SkaldQuery::get_args() const { return args_; }
int SkaldQuery::get_arg_count() const { return args_.size(); }

Variant SkaldQuery::get_arg(int p_index) const {
	if (p_index < 0 || p_index >= args_.size()) {
		return Variant();
	}
	return args_[p_index];
}

// --- SkaldAction ---

void SkaldAction::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_method"), &SkaldAction::get_method);
	ClassDB::bind_method(D_METHOD("get_args"), &SkaldAction::get_args);
	ClassDB::bind_method(D_METHOD("get_arg_count"), &SkaldAction::get_arg_count);
	ClassDB::bind_method(D_METHOD("get_arg", "index"), &SkaldAction::get_arg);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "method"), "", "get_method");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "args"), "", "get_args");
}

void SkaldAction::set_method(const String &p_method) { method_ = p_method; }
void SkaldAction::set_args(const Array &p_args) { args_ = p_args; }

String SkaldAction::get_method() const { return method_; }
Array SkaldAction::get_args() const { return args_; }
int SkaldAction::get_arg_count() const { return args_.size(); }

Variant SkaldAction::get_arg(int p_index) const {
	if (p_index < 0 || p_index >= args_.size()) {
		return Variant();
	}
	return args_[p_index];
}

// --- SkaldExit ---

void SkaldExit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_value"), &SkaldExit::has_value);
	ClassDB::bind_method(D_METHOD("get_value"), &SkaldExit::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, "",
					PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT),
			"", "get_value");
}

void SkaldExit::set_value(const Variant &p_value) { value_ = p_value; }
bool SkaldExit::has_value() const { return value_.get_type() != Variant::NIL; }
Variant SkaldExit::get_value() const { return value_; }

// --- SkaldGoModule ---

void SkaldGoModule::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_module_path"), &SkaldGoModule::get_module_path);
	ClassDB::bind_method(D_METHOD("get_start_tag"), &SkaldGoModule::get_start_tag);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "module_path"), "", "get_module_path");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "start_tag"), "", "get_start_tag");
}

void SkaldGoModule::set_module_path(const String &p_path) { module_path_ = p_path; }
void SkaldGoModule::set_start_tag(const String &p_tag) { start_tag_ = p_tag; }

String SkaldGoModule::get_module_path() const { return module_path_; }
String SkaldGoModule::get_start_tag() const { return start_tag_; }

// --- SkaldEnd ---

void SkaldEnd::_bind_methods() {}

// --- SkaldError ---

void SkaldError::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_code"), &SkaldError::get_code);
	ClassDB::bind_method(D_METHOD("get_message"), &SkaldError::get_message);
	ClassDB::bind_method(D_METHOD("get_line_number"), &SkaldError::get_line_number);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "code"), "", "get_code");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "message"), "", "get_message");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "line_number"), "", "get_line_number");
}

void SkaldError::set_code(int p_code) { code_ = p_code; }
void SkaldError::set_message(const String &p_message) { message_ = p_message; }
void SkaldError::set_line_number(int p_line) { line_number_ = p_line; }

int SkaldError::get_code() const { return code_; }
String SkaldError::get_message() const { return message_; }
int SkaldError::get_line_number() const { return line_number_; }

// --- SkaldParseResult ---

void SkaldParseResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_ok"), &SkaldParseResult::is_ok);
	ClassDB::bind_method(D_METHOD("get_errors"), &SkaldParseResult::get_errors);
	ClassDB::bind_method(D_METHOD("get_error_count"), &SkaldParseResult::get_error_count);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ok"), "", "is_ok");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "errors"), "", "get_errors");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "error_count"), "", "get_error_count");
}

void SkaldParseResult::set_ok(bool p_ok) { ok_ = p_ok; }

void SkaldParseResult::add_error(const String &p_message, int p_line, int p_column,
		const String &p_source, int p_severity) {
	Dictionary err;
	err["message"] = p_message;
	err["line"] = p_line;
	err["column"] = p_column;
	err["source"] = p_source;
	err["severity"] = p_severity;
	errors_.push_back(err);
}

bool SkaldParseResult::is_ok() const { return ok_; }
Array SkaldParseResult::get_errors() const { return errors_; }
int SkaldParseResult::get_error_count() const { return errors_.size(); }

// --- SkaldNotification ---

void SkaldNotification::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_var_name"), &SkaldNotification::get_var_name);
	ClassDB::bind_method(D_METHOD("get_scope"), &SkaldNotification::get_scope);
	ClassDB::bind_method(D_METHOD("has_value"), &SkaldNotification::has_value);
	ClassDB::bind_method(D_METHOD("get_value"), &SkaldNotification::get_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "var_name"), "", "get_var_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "scope"), "", "get_scope");
	ADD_PROPERTY(PropertyInfo(Variant::NIL, "value", PROPERTY_HINT_NONE, "",
					PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT),
			"", "get_value");
}

void SkaldNotification::set_var_name(const String &p_name) { var_name_ = p_name; }
void SkaldNotification::set_scope(const String &p_scope) { scope_ = p_scope; }
void SkaldNotification::set_value(const Variant &p_value) { value_ = p_value; }

String SkaldNotification::get_var_name() const { return var_name_; }
String SkaldNotification::get_scope() const { return scope_; }
bool SkaldNotification::has_value() const { return value_.get_type() != Variant::NIL; }
Variant SkaldNotification::get_value() const { return value_; }
