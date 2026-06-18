#include "skald_engine.h"
#include "skald_responses.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <skald.h>

using namespace godot;

// --- Helpers ---

static Variant rvalue_to_variant(const Skald::RValue &rv) {
	if (auto *s = std::get_if<std::string>(&rv)) {
		return String(s->c_str());
	}
	if (auto *b = std::get_if<bool>(&rv)) {
		return *b;
	}
	if (auto *i = std::get_if<int>(&rv)) {
		return *i;
	}
	if (auto *f = std::get_if<float>(&rv)) {
		return (double)*f;
	}
	if (auto *v = std::get_if<Skald::Variable>(&rv)) {
		return String(v->name.c_str());
	}
	if (auto *mc = std::get_if<std::shared_ptr<Skald::MethodCall>>(&rv)) {
		return String((*mc)->dbg_desc().c_str());
	}
	return Variant();
}

static Variant simple_rvalue_to_variant(const Skald::SimpleRValue &rv) {
	if (auto *s = std::get_if<std::string>(&rv)) {
		return String(s->c_str());
	}
	if (auto *b = std::get_if<bool>(&rv)) {
		return *b;
	}
	if (auto *i = std::get_if<int>(&rv)) {
		return *i;
	}
	if (auto *f = std::get_if<float>(&rv)) {
		return (double)*f;
	}
	return Variant();
}

// Converts a Godot Variant to a Skald SimpleRValue. Returns nullopt for
// unsupported types (anything but bool/int/float/String).
static std::optional<Skald::SimpleRValue> variant_to_simple_rvalue(const Variant &v) {
	switch (v.get_type()) {
		case Variant::BOOL:
			return Skald::SimpleRValue{ (bool)v };
		case Variant::INT:
			return Skald::SimpleRValue{ static_cast<int>(static_cast<int64_t>(v)) };
		case Variant::FLOAT:
			return Skald::SimpleRValue{ static_cast<float>(static_cast<double>(v)) };
		case Variant::STRING: {
			String s = v;
			return Skald::SimpleRValue{ std::string(s.utf8().get_data()) };
		}
		default:
			return std::nullopt;
	}
}

static String chunks_to_string(const std::vector<Skald::Chunk> &chunks) {
	std::string result;
	for (const auto &chunk : chunks) {
		result += chunk.text;
	}
	return String(result.c_str());
}

static Ref<SkaldError> make_error(int code, const String &message, int line_number) {
	Ref<SkaldError> serr;
	serr.instantiate();
	serr->set_code(code);
	serr->set_message(message);
	serr->set_line_number(line_number);
	return serr;
}

static Ref<SkaldParseResult> make_parse_result(const Skald::ParseResult &pr) {
	Ref<SkaldParseResult> spr;
	spr.instantiate();
	spr->set_ok(pr.ok);
	for (const auto &ex : pr.exceptions) {
		spr->add_error(String(ex.msg.c_str()), (int)ex.pos.line, (int)ex.pos.column,
				String(ex.pos.source.c_str()), (int)ex.severity);
	}
	return spr;
}

// Dispatches directly on the 0.6 Response variant. get_response_type() is not
// used: it collapses MethodCallPost / OptionGroup / Notification to UNKNOWN.
static Variant convert_response(Skald::Response &response) {
	return std::visit([](auto &&arg) -> Variant {
		using T = std::decay_t<decltype(arg)>;

		if constexpr (std::is_same_v<T, Skald::Content>) {
			Ref<SkaldContent> sc;
			sc.instantiate();
			sc->set_attribution(String(arg.attribution.c_str()));
			sc->set_text(chunks_to_string(arg.text));
			return sc;
		} else if constexpr (std::is_same_v<T, Skald::OptionGroup>) {
			Ref<SkaldOptionGroup> sg;
			sg.instantiate();
			for (const auto &option : arg.options) {
				sg->add_option(chunks_to_string(option.text), option.is_available);
			}
			return sg;
		} else if constexpr (std::is_same_v<T, Skald::MethodCallGet>) {
			Ref<SkaldQuery> sq;
			sq.instantiate();
			sq->set_method(String(arg.call.method.c_str()));
			Array args;
			for (const auto &a : arg.call.args) {
				args.push_back(rvalue_to_variant(a));
			}
			sq->set_args(args);
			return sq;
		} else if constexpr (std::is_same_v<T, Skald::MethodCallPost>) {
			Ref<SkaldAction> sa;
			sa.instantiate();
			sa->set_method(String(arg.call.method.c_str()));
			Array args;
			for (const auto &a : arg.call.args) {
				args.push_back(rvalue_to_variant(a));
			}
			sa->set_args(args);
			return sa;
		} else if constexpr (std::is_same_v<T, Skald::Exit>) {
			Ref<SkaldExit> se;
			se.instantiate();
			if (arg.argument.has_value()) {
				se->set_value(rvalue_to_variant(arg.argument.value()));
			}
			return se;
		} else if constexpr (std::is_same_v<T, Skald::GoModule>) {
			Ref<SkaldGoModule> sg;
			sg.instantiate();
			sg->set_module_path(String(arg.module_path.c_str()));
			sg->set_start_tag(String(arg.start_in_tag.c_str()));
			return sg;
		} else if constexpr (std::is_same_v<T, Skald::End>) {
			Ref<SkaldEnd> se;
			se.instantiate();
			return se;
		} else if constexpr (std::is_same_v<T, Skald::Error>) {
			return make_error((int)arg.code, String(arg.message.c_str()),
					(int)arg.line_number);
		} else if constexpr (std::is_same_v<T, Skald::Notification>) {
			Ref<SkaldNotification> sn;
			sn.instantiate();
			sn->set_var_name(String(arg.var_name.c_str()));
			sn->set_scope(String(Skald::scope_to_str(arg.scope).c_str()));
			if (arg.rval.has_value()) {
				sn->set_value(simple_rvalue_to_variant(arg.rval.value()));
			}
			return sn;
		} else {
			return Variant();
		}
	}, response);
}

// --- SkaldEngine ---

SkaldEngine::SkaldEngine() : engine_(std::make_unique<Skald::Engine>()) {
	// Route all source reads (initial codex + every GO transition) through
	// Godot's FileAccess so res:// URIs resolve in both editor and exported
	// (.pck) builds, where assets never exist on the OS filesystem.
	engine_->set_source_reader(
			[](const std::string &resolved) -> std::optional<std::string> {
				String gpath = String(resolved.c_str());
				Ref<FileAccess> f = FileAccess::open(gpath, FileAccess::READ);
				if (f.is_null()) {
					return std::nullopt;
				}
				return std::string(f->get_as_text().utf8().get_data());
			});
}
SkaldEngine::~SkaldEngine() = default;

void SkaldEngine::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "path"), &SkaldEngine::setup);
	ClassDB::bind_method(D_METHOD("load", "path"), &SkaldEngine::load);
	ClassDB::bind_method(D_METHOD("start"), &SkaldEngine::start);
	ClassDB::bind_method(D_METHOD("start_at", "tag"), &SkaldEngine::start_at);
	ClassDB::bind_method(D_METHOD("act", "choice_index"), &SkaldEngine::act, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("advance"), &SkaldEngine::continue_);
	ClassDB::bind_method(D_METHOD("get_current"), &SkaldEngine::get_current);
	ClassDB::bind_method(D_METHOD("answer", "value"), &SkaldEngine::answer);
	ClassDB::bind_method(D_METHOD("set_global", "key", "value"), &SkaldEngine::set_global);
	ClassDB::bind_method(D_METHOD("get_global", "key"), &SkaldEngine::get_global);

	ClassDB::bind_method(D_METHOD("set_codex_path", "path"), &SkaldEngine::set_codex_path);
	ClassDB::bind_method(D_METHOD("get_codex_path"), &SkaldEngine::get_codex_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "codex_path", PROPERTY_HINT_FILE, "*.codex"),
			"set_codex_path", "get_codex_path");
}

void SkaldEngine::_ready() {
	// Skip auto-setup in the editor; only load the codex at runtime.
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	if (!codex_path_.is_empty()) {
		setup(codex_path_);
	} else {
		UtilityFunctions::print("No Codex file set in inspector; implement in code with .setup(path) if you would like to use globals or methods.");
	}
}

void SkaldEngine::set_codex_path(const String &p_path) {
	codex_path_ = p_path;
}

String SkaldEngine::get_codex_path() const {
	return codex_path_;
}

Variant SkaldEngine::setup(const String &p_path) {
	Skald::ParseResult result = engine_->setup(std::string(p_path.utf8().get_data()));
	return make_parse_result(result);
}

Variant SkaldEngine::load(const String &p_path) {
	Skald::ParseResult result = engine_->load(std::string(p_path.utf8().get_data()));
	return make_parse_result(result);
}

Variant SkaldEngine::start() {
	Skald::Response response = engine_->start();
	current_response_ = convert_response(response);
	return current_response_;
}

Variant SkaldEngine::start_at(const String &p_tag) {
	Skald::Response response = engine_->start_at(std::string(p_tag.utf8().get_data()));
	current_response_ = convert_response(response);
	return current_response_;
}

Variant SkaldEngine::act(int p_choice_index) {
	Skald::Response response = engine_->act(p_choice_index);
	current_response_ = convert_response(response);
	return current_response_;
}

Variant SkaldEngine::continue_() {
	return act(0);
}

Variant SkaldEngine::get_current() {
	return current_response_;
}

Variant SkaldEngine::answer(const Variant &p_value) {
	std::optional<Skald::QueryAnswer> qa;

	std::optional<Skald::SimpleRValue> srv = variant_to_simple_rvalue(p_value);
	if (srv.has_value()) {
		qa = Skald::QueryAnswer{ srv.value() };
	} else {
		qa = Skald::QueryAnswer{ std::nullopt };
	}

	Skald::Response response = engine_->answer(qa);
	current_response_ = convert_response(response);
	return current_response_;
}

Variant SkaldEngine::set_global(const String &p_key, const Variant &p_value) {
	std::optional<Skald::SimpleRValue> srv = variant_to_simple_rvalue(p_value);
	if (!srv.has_value()) {
		return make_error(Skald::ERROR_TYPE_MISMATCH,
				"set_global only accepts bool, int, float, or String values.", 0);
	}

	std::optional<Skald::Error> err =
			engine_->set(std::string(p_key.utf8().get_data()), srv.value());
	if (err.has_value()) {
		return make_error((int)err->code, String(err->message.c_str()),
				(int)err->line_number);
	}
	return Variant();
}

Variant SkaldEngine::get_global(const String &p_key) {
	std::variant<Skald::Error, Skald::SimpleRValue> result =
			engine_->get(std::string(p_key.utf8().get_data()));
	if (auto *err = std::get_if<Skald::Error>(&result)) {
		return make_error((int)err->code, String(err->message.c_str()),
				(int)err->line_number);
	}
	return simple_rvalue_to_variant(std::get<Skald::SimpleRValue>(result));
}
