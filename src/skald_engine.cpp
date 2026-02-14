#include "skald_engine.h"
#include "skald_responses.h"

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

static String chunks_to_string(const std::vector<Skald::Chunk> &chunks) {
	std::string result;
	for (const auto &chunk : chunks) {
		result += chunk.text;
	}
	return String(result.c_str());
}

static Variant convert_response(Skald::Response &response) {
	Skald::ResponseType type = Skald::get_response_type(response);

	switch (type) {
		case Skald::ResponseType::CONTENT: {
			auto &content = std::get<Skald::Content>(response);
			Ref<SkaldContent> sc;
			sc.instantiate();
			sc->set_attribution(String(content.attribution.c_str()));
			sc->set_text(chunks_to_string(content.text));
			for (const auto &option : content.options) {
				sc->add_option(chunks_to_string(option.text), option.is_available);
			}
			return sc;
		}
		case Skald::ResponseType::QUERY: {
			auto &query = std::get<Skald::Query>(response);
			Ref<SkaldQuery> sq;
			sq.instantiate();
			sq->set_method(String(query.call.method.c_str()));
			Array args;
			for (const auto &arg : query.call.args) {
				args.push_back(rvalue_to_variant(arg));
			}
			sq->set_args(args);
			sq->set_expects_response(query.expects_response);
			return sq;
		}
		case Skald::ResponseType::EXIT: {
			auto &exit = std::get<Skald::Exit>(response);
			Ref<SkaldExit> se;
			se.instantiate();
			if (exit.argument.has_value()) {
				se->set_value(rvalue_to_variant(exit.argument.value()));
			}
			return se;
		}
		case Skald::ResponseType::GO_MODULE: {
			auto &go = std::get<Skald::GoModule>(response);
			Ref<SkaldGoModule> sg;
			sg.instantiate();
			sg->set_module_path(String(go.module_path.c_str()));
			sg->set_start_tag(String(go.start_in_tag.c_str()));
			return sg;
		}
		case Skald::ResponseType::END: {
			Ref<SkaldEnd> se;
			se.instantiate();
			return se;
		}
		case Skald::ResponseType::ERROR: {
			auto &err = std::get<Skald::Error>(response);
			Ref<SkaldError> serr;
			serr.instantiate();
			serr->set_code(err.code);
			serr->set_message(String(err.message.c_str()));
			serr->set_line_number(err.line_number);
			return serr;
		}
		default:
			return Variant();
	}
}

// --- SkaldEngine ---

SkaldEngine::SkaldEngine() : engine_(std::make_unique<Skald::Engine>()) {}
SkaldEngine::~SkaldEngine() = default;

void SkaldEngine::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "path"), &SkaldEngine::load);
	ClassDB::bind_method(D_METHOD("start"), &SkaldEngine::start);
	ClassDB::bind_method(D_METHOD("start_at", "tag"), &SkaldEngine::start_at);
	ClassDB::bind_method(D_METHOD("act", "choice_index"), &SkaldEngine::act, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_current"), &SkaldEngine::get_current);
	ClassDB::bind_method(D_METHOD("answer", "value"), &SkaldEngine::answer);
}

void SkaldEngine::load(const String &p_path) {
	engine_->load(std::string(p_path.utf8().get_data()));
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

Variant SkaldEngine::get_current() {
	return current_response_;
}

Variant SkaldEngine::answer(const Variant &p_value) {
	std::optional<Skald::QueryAnswer> qa;

	switch (p_value.get_type()) {
		case Variant::BOOL:
			qa = Skald::QueryAnswer{ Skald::SimpleRValue{ (bool)p_value } };
			break;
		case Variant::INT:
			qa = Skald::QueryAnswer{ Skald::SimpleRValue{ static_cast<int>(static_cast<int64_t>(p_value)) } };
			break;
		case Variant::FLOAT:
			qa = Skald::QueryAnswer{ Skald::SimpleRValue{ static_cast<float>(static_cast<double>(p_value)) } };
			break;
		case Variant::STRING: {
			String s = p_value;
			qa = Skald::QueryAnswer{ Skald::SimpleRValue{ std::string(s.utf8().get_data()) } };
			break;
		}
		default:
			qa = Skald::QueryAnswer{ std::nullopt };
			break;
	}

	Skald::Response response = engine_->answer(qa);
	current_response_ = convert_response(response);
	return current_response_;
}
