#include "skald_responses.h"

using namespace godot;

// --- SkaldContent ---

void SkaldContent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_attribution"), &SkaldContent::get_attribution);
	ClassDB::bind_method(D_METHOD("get_text"), &SkaldContent::get_text);
	ClassDB::bind_method(D_METHOD("get_option_count"), &SkaldContent::get_option_count);
	ClassDB::bind_method(D_METHOD("get_option_text", "index"), &SkaldContent::get_option_text);
	ClassDB::bind_method(D_METHOD("is_option_available", "index"), &SkaldContent::is_option_available);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "attribution"), "", "get_attribution");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "option_count"), "", "get_option_count");
}

void SkaldContent::set_attribution(const String &p_attribution) { attribution_ = p_attribution; }
void SkaldContent::set_text(const String &p_text) { text_ = p_text; }

void SkaldContent::add_option(const String &p_text, bool p_available) {
	option_texts_.push_back(p_text);
	option_available_.push_back(p_available ? 1 : 0);
}

String SkaldContent::get_attribution() const { return attribution_; }
String SkaldContent::get_text() const { return text_; }
int SkaldContent::get_option_count() const { return option_texts_.size(); }

String SkaldContent::get_option_text(int p_index) const {
	if (p_index < 0 || p_index >= option_texts_.size()) {
		return String();
	}
	return option_texts_[p_index];
}

bool SkaldContent::is_option_available(int p_index) const {
	if (p_index < 0 || p_index >= option_available_.size()) {
		return false;
	}
	return option_available_[p_index] != 0;
}

// --- SkaldQuery ---

void SkaldQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_method"), &SkaldQuery::get_method);
	ClassDB::bind_method(D_METHOD("get_args"), &SkaldQuery::get_args);
	ClassDB::bind_method(D_METHOD("get_arg_count"), &SkaldQuery::get_arg_count);
	ClassDB::bind_method(D_METHOD("get_arg", "index"), &SkaldQuery::get_arg);
	ClassDB::bind_method(D_METHOD("get_expects_response"), &SkaldQuery::get_expects_response);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "method"), "", "get_method");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "args"), "", "get_args");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "expects_response"), "", "get_expects_response");
}

void SkaldQuery::set_method(const String &p_method) { method_ = p_method; }
void SkaldQuery::set_args(const Array &p_args) { args_ = p_args; }
void SkaldQuery::set_expects_response(bool p_expects) { expects_response_ = p_expects; }

String SkaldQuery::get_method() const { return method_; }
Array SkaldQuery::get_args() const { return args_; }
int SkaldQuery::get_arg_count() const { return args_.size(); }

Variant SkaldQuery::get_arg(int p_index) const {
	if (p_index < 0 || p_index >= args_.size()) {
		return Variant();
	}
	return args_[p_index];
}

bool SkaldQuery::get_expects_response() const { return expects_response_; }

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
